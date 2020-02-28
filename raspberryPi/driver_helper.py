import json

# Handles serial transmission of data
def serialPrint(serialObj, data):
    if type(data) in (str, int, float):
        serialObj.write(str(data).encode())
    elif type(data) in (dict, list, tuple):
        serialisedData = json.dumps(dictData)
        serialObj.write(serialisedData.encode())
    else:
        return False
    serialObj.flush()
    return True

# Calculate gradient of a line found
def getGradient(line):
    x1, y1, x2, y2 = line[0]
    return (y1 - y2) / max((x2 - x1), 1)            # max((x2 - x1), 1) prevents division by 0

# Constrain value within given range
def constrain(value, min_value, max_value):
    return min(max_value, max(min_value, value))