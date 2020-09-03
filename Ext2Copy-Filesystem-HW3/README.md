# Ext2Copy

A Program that can copy a regular file with any extention type into an ext2 image without mounting it.

This implementation reads the ext2 image metadata to [ext2 structure](https://www.nongnu.org/ext2-doc/ext2.html) and copies the source file under the target directory as a new file provided with multiple block group and Triple Indirect block support which allows copying files in the gigabyte ranges. 

#### Keywords
```
filesystem, fsck, ext2, inode, double-triple-indirect block
```

## Usage
```
git clone https://github.com/egeozbek/operating-systems-334-2019
cd Ext2Copy-Filesystem-HW3/Source
make all
./filecopy <ext2_image_name> <source_file> <target_path/inode>
```
After the last line, given source file will be copied under given target path in the given ext2 filesystem. As a target,  an existing path in the given image should be give. As an alternative target can be specified as an absolute path in ext2 image or its inode number can be directly given 
(Eg. instead of `/` one can use `2` which corresponds to root inode in ext2 filesystems). 

There are some image and files ready for copying located under ```tests``` folder for testing purposes. 
