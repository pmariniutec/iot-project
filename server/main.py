from yoloface.face_detector import yolodetector
import argparse
import numpy as np
from pil import image

parser = argparse.argumentparser(prog='facecounter', description='count faces using yolo model')
parser.add_argument('filename')
args = parser.parse_args()

model = yolodetector(device="cpu", min_face=40)
orgimg = image.open(args.filename)
img = orgimg.rotate(-90)

imgarray = np.array(img)
bboxes, points = model.predict(imgarray)
people_count = len(bboxes)
print(people_count)
