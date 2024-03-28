from PIL import Image, ImageOps
basepath = "c:\\temp\\"
name = "Farnsworth0"
im = Image.open(basepath+name+".png") # Can be many different formats.
imgs = ImageOps.grayscale(im)
pix = imgs.load()

(xmax, ymax) = im.size   # Get the width and hight of the image for iterating over
print(name, " X", xmax, " y", ymax)
out = open(basepath+name+".c", "w")
out.write("// From "+name+"\n")
out.write("//img_x_max="+str(xmax)+";\n")
out.write("//img_y_max="+str(ymax)+";\n")
out.write(name+" = [")
for x in range(xmax):
    out.write("\n    ")
    b = 0
    bits = 0
    for y in range(ymax):
        value = pix[x, y]  # Get the RGBA Value of the a pixel of an image
        print(value)
        if value > 128:
            bits += 1 ^ b
        if b == 7:
            out.write("0x%0.2X" % bits + ",")
            b = 0
            bits = 0
        else:
            b += 1
out.write(str(bits)+"\n")
out.write("]\n")
out.close()

