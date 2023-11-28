from yoloface.face_detector import YoloDetector
import argparse
import cv2

parser = argparse.ArgumentParser(prog='facecounter', description='count faces using yolo model')
parser.add_argument('filename')
args = parser.parse_args()

model = YoloDetector(device="cuda:0", min_face=20)
image = cv2.imread(args.filename)
# image = cv2.rotate(image, cv2.ROTATE_90_CLOCKWISE)

bboxes, points = model.predict(image)
print(bboxes)

if (len(bboxes) and len(bboxes[0])):
    people_count = len(bboxes[0])
    for bbox in bboxes[0]:
        start = (bbox[0], bbox[1])
        end = (bbox[2], bbox[3])
        print(start, end)
        # TMP: visualize predictions
        image = cv2.rectangle(image, start, end, (0, 0, 255), 3)
    print(f"People: {people_count}")
else:
    print("People: 0")

cv2.imshow("Rectangle", image)
cv2.waitKey(0)
cv2.destroyAllWindows()
