from yoloface.face_detector import YoloDetector
import argparse
import numpy as np
from PIL import Image
import cv2

parser = argparse.ArgumentParser(prog='facecounter', description='count faces using yolo model')
parser.add_argument('filename')
args = parser.parse_args()

model = YoloDetector(device="cuda:0", min_face=40)
src = cv2.imread(args.filename)
i = cv2.rotate(src, cv2.ROTATE_90_CLOCKWISE)
# orgimg = Image.open(args.filename)
# img = orgimg.rotate(-90)
# imgarray = np.array(img)

bboxes, points = model.predict(i)
people_count = len(bboxes)
print(people_count)

if (len(bboxes) and len(bboxes[0])):
    bbox = bboxes[0][0]
    start = (bbox[0], bbox[1])
    end = (bbox[2], bbox[3])
    print(start, end)

    # TMP. visualize predictions
    image = cv2.rectangle(i, start, end, (0, 0, 255), 5)
    cv2.imshow("Rectangle", image)
    cv2.waitKey(0)
    cv2.destroyAllWindows()
