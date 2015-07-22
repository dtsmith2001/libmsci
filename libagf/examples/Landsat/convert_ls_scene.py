#!/usr/bin/python

import sys
import Image
import argparse

parser=argparse.ArgumentParser(description="Convert a Landsat scene into a set of vectors for classification.")
parser.add_argument("fbase", help="Base name for Landsat raster (tiff) image files.")
parser.add_argument("--texture", help="For texture classification: use group of nine pixels.", action="store_true")

args=parser.parse_args()

#fbase=sys.argv[1]
fbase=args.fbase
#outfile=sys.argv[3]

f1=fbase+"_B1.TIF"
f2=fbase+"_B2.TIF"
f3=fbase+"_B3.TIF"
f4=fbase+"_B4.TIF"
f5=fbase+"_B5.TIF"
f6=fbase+"_B6.TIF"
f7=fbase+"_B7.TIF"

ch1=Image.open(f1)

(nx,ny)=ch1.size
#nx=nxbyny[0]
#ny=nxbyny[1]

p1=ch1.load()
ch2=Image.open(f2)
p2=ch2.load()
ch3=Image.open(f3)
p3=ch3.load()
ch4=Image.open(f4)
p4=ch4.load()
ch5=Image.open(f5)
p5=ch5.load()
ch6=Image.open(f6)
p6=ch6.load()
ch7=Image.open(f7)
p7=ch7.load()

#outfs=open(outfile, "w")

for x in range(1, nx-2):
    for y in range(1, ny-2):
        if args.texture:
            print p1[x, y], p2[x, y], p3[x, y], p4[x, y], p5[x, y], p6[x, y], p7[x,y], 
		p1[x-1, y], p2[x-1, y], p3[x-1, y], p4[x-1, y], p5[x-1, y], p6[x-1, y], p7[x-1, y]
		p1[x+1, y], p2[x+1, y], p3[x+1, y], p4[x+1, y], p5[x+1, y], p6[x+1, y], p7[x+1, y]
		p1[x, y-1], p2[x, y-1], p3[x, y-1], p4[x, y-1], p5[x, y-1], p6[x, y-1], p7[x, y-1]
		p1[x, y+1], p2[x, y+1], p3[x, y+1], p4[x, y+1], p5[x, y+1], p6[x, y+1], p7[x, y+1]
		p1[x-1, y-1], p2[x-1, y-1], p3[x-1, y-1], p4[x-1, y-1], p5[x-1, y-1], p6[x-1, y-1], p7[x-1, y-1]
		p1[x-1, y+1], p2[x-1, y+1], p3[x-1, y+1], p4[x-1, y+1], p5[x-1, y+1], p6[x-1, y+1], p7[x-1, y+1]
		p1[x+1, y-1], p2[x+1, y-1], p3[x+1, y-1], p4[x+1, y-1], p5[x+1, y-1], p6[x+1, y-1], p7[x+1, y-1]
		p1[x+1, y+1], p2[x+1, y+1], p3[x+1, y+1], p4[x+1, y+1], p5[x+1, y+1], p6[x+1, y+1], p7[x+1, y+1]
        else:
            print p1[x, y], p2[x, y], p3[x, y], p4[x, y], p5[x, y], p6[x, y], p7[x,y]

#outfs.close()

