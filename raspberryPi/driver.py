import serial, time, cv2, numpy as np
from driver_helper import *

# Control instructions to be sent to ESP32 in JSON form
controlInfo = {"target RPM": 0, "steering": 90, "camera tilt": 90, "camera pan": 90}
# ESP32 serial communication
serialESP = serial.Serial("/dev/ttyS0", baudrate=115200, timeout=0.02)

# Handles initial establishment of communications with ESP32
def initComms(timeout):
    uart_received = ""
    # serialESP.write("Pi Ready.".encode())                       # Send "Pi Ready" signal indicating it has booted and running the code
    # serialESP.flush()
    serialPrint(serialESP, "Pi Ready.")

    # Wait for "OK" response from ESP32 within timeout period, thereby confirming it recieves messages from the Pi
    start_time = time.time()                                    # Start timestamp for timeout check
    while uart_received != "OK":
        if time.time() - start_time >= timeout:
            return False                                        # Indicate communication failed
        else:
            uart_received = serialESP.read(2).decode("utf-8")
            time.sleep(0.1)
    
    # Wait for "ESP Ready" signal within timeout period, indicating ESP32 is ready to run main code
    while uart_received != "ESP Ready.":
        if time.time() - start_time >= timeout:
            return False                                        # Indicate communication failed
        else:
            uart_received = serialESP.read(30).decode("utf-8")
            time.sleep(0.1)

    # serialESP.write("OK".encode())                              # Respond with OK, thereby confirming Pi recieves ESP32 messages
    # serialESP.flush()
    serialPrint(serialESP, "OK")
    return True                                                 # Indicate communication succesful

# Establish and synchronise communications between ESP32 and Raspberry Pi
if initComms(3):
    print("UART Communication with ESP32 established!")
else:
    print("UART Handshake with ESP32 failed...")
    serialESP.close()
    exit

camera = cv2.VideoCapture(0)        # Select Pi Camera as Video Source
camera.set(3, 640)                  # Set frame width to 640px
camera.set(4, 480)                  # Set frame height to 480px
camera.set(5, 40)                   # Set frame rate to 40 FPS

# Frame count and timestamp for FPS measurement
frameCount = 0
timestamp = time.time()

# Main program loop
while True:
    if frameCount % 10 == 0:
        print("FPS: {}".format(10 / (time.time() - timestamp)))                 # Output average FPS (averaged over 10 frames)
        timestamp = time.time( )                                                # Set timestamp to now
    
    # ------------------ Image Pre-Processing ------------------ #
    _, frame = camera.read()                                                    # Read frame from camera
    grayscale = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)                         # Convert frame to grayscale
    grayscale = cv2.resize(grayscale, None, fx=0.5, fy=0.5, interpolation=cv2.INTER_AREA)

    ROI_lane = grayscale[110:, :]                                               # ROI for lane lines
    edgemap = cv2.Canny(ROI_lane, 150, 180)                                     # Edge detection on lane lines ROI

    signs_rightROI = grayscale[:, 200:]                                         # Right side ROI for road signs
    signs_leftROI = grayscale[:, :100]                                          # Left side ROI for road signs

    # --------------------- Lane Detection --------------------- #
    ROI_lane = cv2.cvtColor(ROI_lane, cv2.COLOR_GRAY2BGR)                       # Convert ROI back to 3 channel colour image for drawing coloured lines
    # Find stright lines in edge map of Lane ROI
    lines = cv2.HoughLinesP(edgemap, 1, np.pi/180, 30, minLineLength=40, maxLineGap=20)
    
    if isinstance(lines, np.ndarray):                                           # Check if any lines are detected
        gradients = list(map(getGradient, lines))                               # Get list of gradients of all detected lines
        
        # Find all lines from detected lines that may belong to the left lane and meet gradient threshold
        left_lines = [line[0] for gradient, line in zip(gradients, lines) if gradient > 0.3]
        # Create list of indexes of all lines in 'left_lines' that intersect the left/bottom boundary of the image
        leftBoundaryLines_idx = [lineIndex for lineIndex in range(len(left_lines)) if (left_lines[lineIndex][0] <= 5 or left_lines[lineIndex][1] >= ROI_lane.shape[0] - 5)]
        # Find all lines from detected lines that may belong to the right lane and meet gradient threshold
        right_lines = [line[0] for gradient, line in zip(gradients, lines) if gradient < -0.3]
        # Create list of indexes of all lines in 'right_lines' that intersect the right/bottom boundary of the image
        rightBoundaryLines_idx = [lineIndex for lineIndex in range(len(right_lines)) if (right_lines[lineIndex][2] >= ROI_lane.shape[1] - 5 or right_lines[lineIndex][3] >= ROI_lane.shape[0] - 5)]
        
        validLeft_x, validLeft_y = [], []                                       # To hold x and y coordinate values of valid left lines separately
        validRight_x, validRight_y = [], []                                     # To hold x and y coordinate values of valid right lines separately
        overlay = np.zeros_like(ROI_lane)                                       # Overlay image to be overlayed onto lane line ROI

        # If multiple left boundary intersecting lines found, filter by adaptive threshold
        if len(leftBoundaryLines_idx) > 1:
            # Generate list of left boundary intersecting lines using indexes in 'leftBoundaryLine_idx'
            leftBoundaryLines = [left_lines[idx] for idx in leftBoundaryLines_idx]
            # Create left intersection thresholds along x-axis and y-axis
            left_threshold_x = constrain(max(left[0] for left in leftBoundaryLines) - 25, 0, ROI_lane.shape[1])
            left_threshold_y = max([left[1] for left in leftBoundaryLines]) - 25
            # Display threshold point as a green plus
            cv2.drawMarker(ROI_lane, (left_threshold_x, left_threshold_y), (0, 255, 0), cv2.MARKER_CROSS, 12, 1)
            # Display all valid left boundary intersecting lines on lane ROI
            for x1, y1, x2, y2 in list(filter(lambda line: line[1] >= left_threshold_y and line[0] >= left_threshold_x, leftBoundaryLines)):
                validLeft_x.extend([x1, x2])
                validLeft_y.extend([y1, y2])

        else:
            if len(leftBoundaryLines_idx) == 1:
                del left_lines[leftBoundaryLines_idx[0]]                        # If only 1 left boundary intersecting line, remove this line
            # Display all valid lines on lane ROI. These lines are valid but dont intersect the boundary.
            for x1, y1, x2, y2 in list(filter(lambda line: line[0] < (ROI_lane.shape[1]/2), left_lines)):
                validLeft_x.extend([x1, x2])
                validLeft_y.extend([y1, y2])           

        # (Perform same validation logic on right lines)
        if len(rightBoundaryLines_idx) > 1:
            rightBoundaryLines = [right_lines[idx] for idx in rightBoundaryLines_idx]
            right_threshold_x = constrain(min([right[2] for right in rightBoundaryLines]) + 25, 0, ROI_lane.shape[1])
            right_threshold_y = max([right[3] for right in rightBoundaryLines]) - 25
            cv2.drawMarker(ROI_lane, (right_threshold_x, right_threshold_y), (0, 0, 255), cv2.MARKER_CROSS, 12, 1)
            for x1, y1, x2, y2 in list(filter(lambda line: line[3] >= right_threshold_y and line[2] <= right_threshold_x, rightBoundaryLines)):
                validRight_x.extend([x1, x2])
                validRight_y.extend([y1, y2])
        else:
            if len(rightBoundaryLines_idx) == 1:
                del right_lines[rightBoundaryLines_idx[0]]
            
            for x1, y1, x2, y2 in list(filter(lambda line: line[2] > (ROI_lane.shape[1]/2), right_lines)):
                validRight_x.extend([x1, x2])
                validRight_y.extend([y1, y2])
        
        if len(validLeft_x) >= 2 and len(validLeft_y) >= 2:
            leftLane = np.poly1d(np.polyfit(validLeft_y, validLeft_x, 1))       # If valid left lines were found, approximate left lane line
            # Display left lane line on overlay
            start_y, end_y = max(validLeft_y), min(validLeft_y)
            cv2.line(overlay, (int(leftLane(start_y)), start_y), (int(leftLane(end_y)), end_y), (0, 255, 0), 4) 
        if len(validRight_x) >= 2 and len(validRight_y) >= 2:
            rightLane = np.poly1d(np.polyfit(validRight_y, validRight_x, 1))    # If valid right lines were found, approximate right lane line
            start_y, end_y = max(validRight_y), min(validRight_y)
            # Display right lane line on overlay
            cv2.line(overlay, (int(rightLane(start_y)), start_y), (int(rightLane(end_y)), end_y), (0, 0, 255), 4)

        ROI_lane = cv2.addWeighted(ROI_lane, 1.0, overlay, 0.5, 0)              # Add overlay onto lane line ROI
    
    # ---------------- Image Output for Testing ---------------- #
    # cv2.imshow("Grayscale", grayscale)		                                    # Display grayscale of frame
    cv2.imshow("ROI lane", ROI_lane)                                            # Display ROI for lane lines
    cv2.imshow("ROI lane edgemap", edgemap)                                     # Display edge map of lane lines
    # cv2.imshow("signs on the Right", signs_rightROI)                            # Display ROI for signs on the right
    # cv2.imshow("signs on the left", signs_leftROI)                              # Display ROI for signs on the left

    key = cv2.waitKey(1) & 0xFF		                                            # Read keypress (if any key is pressed)
    if key == ord("q"):				                                            # If `q` key was pressed, break from the loop
        break

    frameCount += 1                                                             # Increment frame count


camera.release()					# Release camera
cv2.destroyAllWindows()				# Close all program windows