from PIL import Image
import numpy as np
import os

in_dir = './images'
out_dir = './resource'

for filename in os.listdir(in_dir):
    path = os.path.join(in_dir, filename)
    if not path.endswith(".jpg"):
        continue
    image = Image.open(path)
    data = np.asarray(image, dtype='<u1')
    height, width, color_dim = data.shape
    if height > 480 or width > 640:
        print("{} ({}x{}) is too large. ignored", filename)
        continue

    filename_no_ext = os.path.splitext(filename)[0]
    new_filename = "{}/{}.img".format(out_dir, filename_no_ext)
    with open(new_filename, "wb") as f:
        f.write(width.to_bytes(4, 'little'));
        f.write(height.to_bytes(4, 'little'));
        f.write(data.tobytes())
        print("{} ({}x{}) was generated".format(new_filename, width, height))
