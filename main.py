from yoloface.face_detector import YoloDetector
import argparse
import numpy as np
from PIL import Image

parser = argparse.ArgumentParser(prog='FaceCounter', description='Count faces using YOLO model')
parser.add_argument('filename')
args = parser.parse_args()

model = YoloDetector(device="cuda:0", min_face=40)
orgimg = Image.open(args.filename)
img = orgimg.rotate(-90)

imgarray = np.array(img)
bboxes, points = model.predict(imgarray)
print(bboxes)
print(points)
print(len(points))
