# import the necessary packages
import cv2, numpy

picNum = 1							# Picture number
cap = cv2.VideoCapture(0)			# Camera source
while True:
	_, image = cap.read()			# Read frame from camera
	cv2.imshow("Frame", image)		# Display frame

	key = cv2.waitKey(1) & 0xFF		# Read keypress (if any key is pressed)
	if key == ord("q"):				# If `q` key was pressed, break from the loop
		break
	elif key == ord(" "):			# If SPACE was pressed, save image
		fileName = "calib_" + str(picNum) + ".jpg"
		cv2.imwrite(fileName, image)
		print("Image ", fileName, " saved!")
		picNum += 1					# Increment picture number after saving, so that next save doesnt overwrite previous save

cap.release()						# Release camera source
cv2.destroyAllWindows()				# Close all program windows