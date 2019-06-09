import os
import struct

with open('test_initrd', 'wb') as outfile:
    files = os.listdir('initfs/root') # TODO: support directories later
    os.chdir('initfs/root')
    print(files)
    for file in files:
        print(file)
        # Write the size and name of the file
        size = os.path.getsize(file)
        print(size)
        outfile.write(struct.pack('I', size))
        outfile.write(struct.pack('128s', bytes(file, 'utf-8')))
        with open(file, 'rb') as infile:
            outfile.write(infile.read())
