#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "ext2.h"
#include <sys/stat.h>
#include <string.h>
#include <libgen.h>

#define BASE_OFFSET 1024
#define EXT2_BLOCK_SIZE 1024
#define IMAGE "image.img"

typedef unsigned char bmap;
#define __NBITS (8 * (int) sizeof (bmap))
#define __BMELT(d) ((d) / __NBITS)
#define __BMMASK(d) ((bmap) 1 << ((d) % __NBITS))
#define BM_SET(d, set) ((set[__BMELT (d)] |= __BMMASK (d)))
#define BM_CLR(d, set) ((set[__BMELT (d)] &= ~__BMMASK (d)))
#define BM_ISSET(d, set) ((set[__BMELT (d)] & __BMMASK (d)) != 0)

unsigned int block_size = 0;
unsigned int number_of_block_groups = 0 ;
unsigned int number_of_data_blocks_per_group = 0;
// #define BLOCK_OFFSET(block) (BASE_OFFSET + (block-1)*block_size)
#define BLOCK_OFFSET(block) (block*block_size)

void changeInodeBitmaps(char * inodeBitmaps,int fd, struct ext2_super_block* super,struct ext2_group_desc * group )
{
  for(unsigned int i = 0;i<number_of_block_groups;i++)
  {
    lseek(fd,BLOCK_OFFSET(group[i].bg_inode_bitmap),SEEK_SET);
    read(fd,inodeBitmaps[i],(block_size));
  }
}
void changeBlockBitmaps(char * blockBitmaps,int fd, struct ext2_super_block* super,struct ext2_group_desc * group )
{
  for(unsigned int i = 0;i<number_of_block_groups;i++)
  {
    lseek(fd,BLOCK_OFFSET(group[i].bg_block_bitmap),SEEK_SET);
    read(fd,blockBitmaps[i],(block_size));
  }
}

void dumpGroupZeroInodeBitmap(int fd,struct ext2_super_block * super, struct ext2_group_desc * group)
{
  printf("-----------Bitmap 0-------------\n");
  bmap tempBitmap[block_size];
  lseek(fd, BLOCK_OFFSET(group[0].bg_inode_bitmap), SEEK_SET);
  read(fd, tempBitmap, block_size);
  for(int i = 0;i<super->s_inodes_per_group/8;i++)
  {
    printf("%u ",(unsigned char)tempBitmap[i]);
    if(i%32==0)
    {
      printf("\n");
    }
  }
  printf("\n");

  printf("-----------Bitmap 1-------------\n");

  lseek(fd, BLOCK_OFFSET(group[1].bg_inode_bitmap), SEEK_SET);
  read(fd, tempBitmap, block_size);
  for(int i = 0;i<super->s_inodes_per_group/8;i++)
  {
    printf("%u ",(unsigned char)tempBitmap[i]);
    if(i%32==0)
    {
      printf("\n");
    }
  }
  printf("\n");

  printf("-----------Bitmap 2-------------\n");

  lseek(fd, BLOCK_OFFSET(group[2].bg_inode_bitmap), SEEK_SET);
  read(fd, tempBitmap, block_size);
  for(int i = 0;i<super->s_inodes_per_group/8;i++)
  {
    printf("%u ",(unsigned char)tempBitmap[i]);
    if(i%32==0)
    {
      printf("\n");
    }
  }
  printf("\n");
}

struct SearchResult
{
  unsigned int ID;
  unsigned int blockGroupNo;
  unsigned int eightBitGroupNo;
  unsigned int bitNo;
};
typedef struct SearchResult SearchResult;

unsigned int toWhichEightBitGroup(unsigned int inodeID,struct ext2_super_block * super)
{
  /*assume inode_per_group is 10, inode with id 10 is in first group, but 11 is in second*/
  printf("toWhichEightBitGroup is called...\n");
  unsigned int temp_calculation = (inodeID)/8;
  if(inodeID%8 == 0)
  {
    printf("toWhichEightBitGroup is in %u...\n\n",temp_calculation);
    return temp_calculation; //inode with id 10
  }
  else
  {
    printf("toWhichEightBitGroup is in %u...\n\n",temp_calculation+1);
    return temp_calculation+1;
  }
}

unsigned int toBitmapIndex(unsigned int indexNo,struct ext2_super_block * super)
{
  printf("In toBitmapIndex , indexNo is %u \n", indexNo );
  unsigned int eightBitGrouping = toWhichEightBitGroup(indexNo,super);
  printf("EightBitGrouping is %u \n", eightBitGrouping );
  unsigned int whichIndexInBitmap = 8*eightBitGrouping - indexNo;
  printf("WhichIndexInBitmap is %u \n", whichIndexInBitmap );
  return whichIndexInBitmap;
}

unsigned int bitMapMask(unsigned int index)
{
  switch(index)
  {
    case 0:
    {
      return 0x80;
      break;
    }
    case 1:
    {
      return 0x40;
      break;
    }
    case 2:
    {
      return 0x20;
      break;
    }
    case 3:
    {
      return 0x10;
      break;
    }
    case 4:
    {
      return 0x08;
      break;
    }
    case 5:
    {
      return 0x04;
      break;
    }
    case 6:
    {
      return 0x02;
      break;
    }
    case 7:
    {
      return 0x01;
      break;
    }

    default:
    {
      printf("Default shoudln't fire!");
      break;
    }
  }
}


unsigned int getModdedTargetInode(unsigned int unmoddedTargetInode, struct ext2_super_block * super)
{
  unsigned int moddedTargetID;
  if(unmoddedTargetInode == 0)
  {
    printf("Target inode id MUSTN'T BE 0\n" );
    exit(-1);
  }
  if(unmoddedTargetInode % super->s_inodes_per_group == 0) //a multiple of inode per group, itself must be result
  {
    moddedTargetID = super->s_inodes_per_group;
  }
  else
  {
    moddedTargetID = unmoddedTargetInode % super->s_inodes_per_group ;
  }
  return moddedTargetID;
}

unsigned int getModdedTargetDataBlock(unsigned int unmoddedTargetDataBlock, struct ext2_super_block * super)
{
  unsigned int moddedTargetID;
  if(unmoddedTargetDataBlock == 0)
  {
    printf("Target datablock id MUSTN'T BE 0\n" );
    exit(-1);
  }
  if(unmoddedTargetDataBlock % super->s_blocks_per_group == 0) //a multiple of inode per group, itself must be result
  {
    moddedTargetID = super->s_blocks_per_group;
  }
  else
  {
    moddedTargetID = unmoddedTargetDataBlock % super->s_blocks_per_group ;
  }
  return moddedTargetID;
}

SearchResult findNextFreeInode(int fd,struct ext2_super_block* super,struct ext2_group_desc * group)
{
  //TODO may speed up by checking free inode count from group descriptor instead of linear seach
  unsigned int foundFlag = 0;
  SearchResult result;
  for(unsigned int p = 0; p< number_of_block_groups;p++)
  {
    if(group[p].bg_free_inodes_count!=0)
    {
      // printf("Block offset is %u\n",(unsigned int) BLOCK_OFFSET(group[p].bg_inode_bitmap) );
      unsigned int eightBitInodeGroupingPerBlockGroup = super->s_inodes_per_group/8;
      unsigned char InodeBitmap;
      lseek(fd,BLOCK_OFFSET(group[p].bg_inode_bitmap),SEEK_SET);
      for(unsigned int k = 0 ; k < eightBitInodeGroupingPerBlockGroup; k++ )
      {
        read(fd, &InodeBitmap, sizeof(InodeBitmap));
        if(InodeBitmap != 255)
        {
          // printf("Found a free inode at block group %u, at %u'th 8bit group.\n",p+1,k+1 );
          printf("Bit Map is %u\n", InodeBitmap );
          unsigned int temp=InodeBitmap;
          for(unsigned int t = 0; t<8;t++)
          {
            if(temp & (unsigned char) 0x1)
            {
              temp = (temp)>>1;
            }
            else
            {
              printf("Found a free inode at block group %u, at %u'th 8bit group, at %u'th bit.\n",p+1,k+1,t+1 );
              foundFlag=1;
              result.blockGroupNo = p+1;
              result.eightBitGroupNo = k+1;
              result.bitNo = t+1;
              result.ID=(super->s_inodes_per_group*p)+(result.eightBitGroupNo-1)*8+result.bitNo;
              printf("Free Inode blockGroupNo is %u\n", result.blockGroupNo );
              printf("Free Inode ID is %u\n\n", result.ID );
              break;
            }
          }
          break;
        }
      }
      if(foundFlag)
        break;
    }
    else
    {
      // printf("No free inode in block group %u\n",p+1 );
    }
  }
  return result;
}
void markInodeAsSetInBitmap(int imageFD,struct ext2_super_block* super,struct ext2_group_desc * group, SearchResult * foundInode,char ** inodeBitmaps)
{
  printf("Marking Inode as Set in Bitmap\n");
  unsigned int moddedTargetID = getModdedTargetInode(foundInode->ID,super);
  printf("Inode block group is %u \n",foundInode->blockGroupNo);
  printf("UnModded Target ID is %u \n",foundInode->ID);
  printf("Modded Target ID is %u \n",moddedTargetID);

  printf("Inode Map Before Marked As Set! %u\n",(unsigned int)inodeBitmaps[foundInode->blockGroupNo-1][foundInode->eightBitGroupNo-1] );
  unsigned int bitMapIndex = toBitmapIndex(moddedTargetID,super);
  printf("Bitmap Index is %u \n",bitMapIndex );
  unsigned int bitMask = bitMapMask(bitMapIndex);
  printf("Bitmap Mask is %u \n",bitMask );
  unsigned int temp = inodeBitmaps[foundInode->blockGroupNo-1][foundInode->eightBitGroupNo-1];
  printf("Temp bitmap is %u \n",temp );
  temp = bitMask | temp;
  printf("Or'd version is %u\n", temp);
  inodeBitmaps[foundInode->blockGroupNo-1][foundInode->eightBitGroupNo-1] = temp;
  printf("Inode Map After Marked As Set! %u\n\n",(unsigned int)inodeBitmaps[foundInode->blockGroupNo-1][foundInode->eightBitGroupNo-1] );

  for(int i = 0;i<super->s_inodes_per_group/8;i++)
  {
    printf("%u ",(unsigned char)inodeBitmaps[foundInode->blockGroupNo-1][foundInode->eightBitGroupNo-1]);
    if(i%32==0)
    {
      printf("\n");
    }
  }
  printf("\n");
}

void markDataBlockAsSetInBitmap(int imageFD,struct ext2_super_block* super,struct ext2_group_desc * group, SearchResult * foundDataBlock,char ** blockBitmaps)
{
  printf("Marking Data Block as Set in Bitmap\n");

  unsigned int moddedTargetID = getModdedTargetDataBlock(foundDataBlock->ID,super);
  printf("DataBlock Group block group is %u \n",foundDataBlock->blockGroupNo);
  printf("UnModded Target ID is %u \n",foundDataBlock->ID);
  printf("Modded Target ID is %u \n",moddedTargetID);

  printf("DataBlock Map Before Marked As Set! %u\n",(unsigned int)blockBitmaps[foundDataBlock->blockGroupNo-1][foundDataBlock->eightBitGroupNo-1] );
  unsigned int bitMapIndex = toBitmapIndex(moddedTargetID,super);
  printf("Bitmap Index is %u \n",bitMapIndex );
  unsigned int bitMask = bitMapMask(bitMapIndex);
  printf("Bitmap Mask is %u \n",bitMask );
  unsigned int temp = blockBitmaps[foundDataBlock->blockGroupNo-1][foundDataBlock->eightBitGroupNo-1];
  printf("Temp bitmap is %u \n",temp );
  temp = bitMask | temp;
  printf("Or'd version is %u\n", temp);


  blockBitmaps[foundDataBlock->blockGroupNo-1][foundDataBlock->eightBitGroupNo-1]=temp;

  for(int i = 0;i<super->s_blocks_per_group/8;i++)
  {
    printf("%u ",(unsigned char)blockBitmaps[foundDataBlock->blockGroupNo-1][foundDataBlock->eightBitGroupNo-1]);
    if(i%32==0)
    {
      printf("\n");
    }
  }
  printf("\n");

  printf("DataBlock Map Marked As Set! %u\n\n",(unsigned int)blockBitmaps[foundDataBlock->blockGroupNo-1][foundDataBlock->eightBitGroupNo-1] );

}

SearchResult findNextFreeDataBlock(int fd,struct ext2_super_block* super,struct ext2_group_desc * group)
{
  unsigned int foundFlag = 0;
  SearchResult result;
  for(unsigned int p = 0; p< number_of_block_groups;p++)
  {
    if(group[p].bg_free_blocks_count!=0)
    {
      // printf("Block offset is %u\n",(unsigned int) BLOCK_OFFSET(group[p].bg_block_bitmap) );
      unsigned int eightBitInodeGroupingPerBlockGroup = super->s_blocks_per_group/8;
      unsigned char DataBitMap;
      lseek(fd,BLOCK_OFFSET(group[p].bg_block_bitmap),SEEK_SET);
      for(unsigned int k = 0 ; k < eightBitInodeGroupingPerBlockGroup; k++ )
      {
        read(fd, &DataBitMap, sizeof(DataBitMap));
        if(DataBitMap != 255)
        {
          // printf("Found a datablock at block group %u, at %u'th 8bit group.\n",p+1,k+1 );
          printf("Bit Map is %u\n", DataBitMap );
          unsigned int temp=DataBitMap;
          for(unsigned int t = 0; t<8;t++)
          {
            if(temp & (unsigned char) 0x1)
            {
              temp = (temp)>>1;
            }
            else
            {
              printf("Found a free data block at block group %u, at %u'th 8bit group, at %u'th bit.\n",p+1,k+1,t+1 );
              foundFlag=1;
              result.blockGroupNo = p+1;
              result.eightBitGroupNo = k+1;
              result.bitNo = t+1;
              result.ID=(super->s_blocks_per_group*p)+(result.eightBitGroupNo-1)*8+result.bitNo;
              printf("Free DataBlock blockGroupNo is %u\n", result.blockGroupNo );
              printf("Free DataBlock ID is %u\n\n", result.ID );
              break;
            }
          }
          break;
        }
        else
        {
          //printf("Bitmap is not free at block group %u , eight Bit Grouping %u \n",p+1,k+1 );
        }
      }
      if(foundFlag)
        break;
    }
    else
    {
      printf("No datablock in block group %u\n",p+1 );
    }

  }
  return result;
}

void readToInode(struct ext2_inode * inodeptr, int fd,struct ext2_group_desc * group ,unsigned int groupNo,unsigned int inodeID)
{
  // printf("**********\n");
  printf("Reading into Inode\n");
  unsigned int inodeTableOffset = group[groupNo-1].bg_inode_table;
  // printf("inode table Offset is %u\n", inodeTableOffset );
  // printf("inode table offset after block offset is %u\n",BLOCK_OFFSET(inodeTableOffset) );
  unsigned int jumpDestination = BLOCK_OFFSET(inodeTableOffset)+sizeof(struct ext2_inode)*(inodeID-1); //assuming it doesnt start from 0
  // printf("inode jumpDestination is %u\n", jumpDestination );
  lseek(fd,jumpDestination,SEEK_SET);
  read(fd,inodeptr,sizeof(struct ext2_inode));
  // printf("**********\n");
}

void copyMetadata(struct ext2_inode * targetDirectoryInode , struct stat * metadataOfSource)
{
  printf("Copying Metadata\n");
  targetDirectoryInode->i_mode = metadataOfSource->st_mode;
  targetDirectoryInode->i_uid = metadataOfSource->st_uid;
  targetDirectoryInode->i_gid = metadataOfSource->st_gid;
  targetDirectoryInode->i_size = metadataOfSource->st_size;
  //direkt st_size i 512 ye bölünde oluyormus

  targetDirectoryInode->i_blocks = ceil((float)metadataOfSource->st_size/block_size)*block_size/512;
  targetDirectoryInode->i_links_count = metadataOfSource->st_nlink;
  targetDirectoryInode->i_atime = metadataOfSource->st_atime;
  targetDirectoryInode->i_mtime = metadataOfSource->st_mtime;
  targetDirectoryInode->i_ctime = metadataOfSource->st_ctime;
}

int readSuperBlock(int fd, struct ext2_super_block* super)
{
    printf("Reading into SuperBlock\n");
    lseek(fd, BASE_OFFSET, SEEK_SET);
    read(fd, super, sizeof(struct ext2_super_block));
    return super->s_magic == EXT2_SUPER_MAGIC;
}
int readGroupDescriptors(int fd, struct ext2_super_block* super,struct ext2_group_desc * group )
{
    printf("Reading into Group descriptors\n");
    lseek(fd, BASE_OFFSET+sizeof(struct ext2_super_block), SEEK_SET);
    // printf("%u is the size of group descriptor\n", (unsigned int) sizeof(*group) );
    read(fd, group, sizeof(struct ext2_group_desc)*number_of_block_groups);
    printf("1Bitmap Inode Table Location %u\n", group[0].bg_inode_table);
    printf("1Bitmap Block Bitmap Location %u\n", group[0].bg_block_bitmap);
    printf("1Bitmap Inode Bitmap Location %u\n", group[0].bg_inode_bitmap);
    printf("2Bitmap Inode Table Location %u\n", group[1].bg_inode_table);
    printf("2Bitmap Block Bitmap Location %u\n", group[1].bg_block_bitmap);
    printf("2Bitmap Inode Bitmap Location %u\n", group[1].bg_inode_bitmap);
    printf("3Bitmap Inode Table Location %u\n", group[2].bg_inode_table);
    printf("3Bitmap Block Bitmap Location %u\n", group[2].bg_block_bitmap);
    printf("3Bitmap Inode Bitmap Location %u\n", group[2].bg_inode_bitmap);

}

int isTargetInode(char * p)
{
  while (*p != '\0')
  {
      if (*p<'0' || *p>'9')
      {
          return 0;
      }
      p++;
  }
  return 1;
}

unsigned int InodeAtWhichBlockGroup(unsigned int inodeID,struct ext2_super_block * super)
{
  /*assume inode_per_group is 10, inode with id 10 is in first group, but 11 is in second*/
  printf("Inode is at which block group...\n");
  unsigned int temp_calculation = (inodeID)/super->s_inodes_per_group;
  if(inodeID%super->s_inodes_per_group == 0)
  {
    printf("Inode is at block group... %u \n",temp_calculation);
    printf("\n" );

    return temp_calculation; //inode with id 10
  }
  else
  {
    printf("Inode is at block group... %u \n",temp_calculation+1);
    printf("\n" );

    return temp_calculation+1;
  }
}
unsigned int DataBlockAtWhichBlockGroup(unsigned int dataBlockID,struct ext2_super_block * super)
{
  /*assume inode_per_group is 10, inode with id 10 is in first group, but 11 is in second*/
  printf("Data Block is at which block group...\n");
  unsigned int temp_calculation = (dataBlockID)/super->s_blocks_per_group;
  if(dataBlockID%super->s_blocks_per_group == 0)
  {
    printf("Data Block is at block group... %u \n",temp_calculation);
    printf("\n" );

    return temp_calculation; //inode with id 10
  }
  else
  {
    printf("Inode is at block group... %u \n",temp_calculation+1);
    printf("\n" );

    return temp_calculation+1;
  }
}

void changeBlockSize(unsigned int * block_size,struct ext2_super_block * super)
{
  printf("Changing Block Size... \n");
  *block_size=ceil(pow(2,10+super->s_log_block_size));
  printf("New Block Size is %u \n",*block_size);
  printf("\n" );


}
void changeNumberOfDataBlocks(unsigned int *number_of_data_blocks_per_group,struct ext2_super_block * super,struct ext2_group_desc * group)
{
  //TODO THIS MAY NEED TO CHANGE
  printf("Changing Number Of Data Blocks Per Group... \n");
  unsigned int beginOfDataBlocks = BLOCK_OFFSET(group[0].bg_inode_table)+sizeof(struct ext2_inode)*(super->s_inodes_per_group+1);
  unsigned int endOfDataBlocks = BLOCK_OFFSET((group[1].bg_inode_table) -2)-(sizeof(struct ext2_group_desc)*number_of_block_groups)-sizeof(struct ext2_super_block);
  *number_of_data_blocks_per_group = (endOfDataBlocks-beginOfDataBlocks)/block_size;
  printf("Number of Data Blocks Per Group is %u \n",*number_of_data_blocks_per_group);
  printf("\n" );

}
void changeNumberOfBlockGroups(unsigned int * number_of_block_groups,struct ext2_super_block * super)
{
  printf("Changing Number of Block Groups... \n");
  *number_of_block_groups=ceil(super->s_inodes_count/super->s_inodes_per_group);
  printf("Number of Block Groups is ... %u \n",*number_of_block_groups);
  printf("\n" );
}

int main(int argc, char **argv)
{
    char * imageFilePath = argv[1];
    char * sourceFilePath = argv[2];
    char * targetPathOrTargetInode = argv[3];
    char ** pathTokenizer;
    struct stat metadataOfSource;
    unsigned int targetInode;
    unsigned int imageFD;
    unsigned int sourceFD;
    struct ext2_super_block super;
    stat(sourceFilePath,&metadataOfSource);

    imageFD = open(imageFilePath,O_RDWR);
    sourceFD = open(sourceFilePath,O_RDWR);

    int isEXT2Image=readSuperBlock(imageFD,&super);
    if(isEXT2Image)
    {
      printf("OPENED EXT2 IMAGE\n");
    }
    else
    {
      printf("IS NOT AN EXT2 IMAGE! EXITING");
      exit(-1);
    }
    struct ext2_inode targetDirectoryInode;

    changeBlockSize(&block_size,&super);
    changeNumberOfBlockGroups(&number_of_block_groups,&super);
    struct ext2_group_desc group[number_of_block_groups];
    readGroupDescriptors(imageFD,&super,group);
    printf("Before begin!\n");
    dumpGroupZeroInodeBitmap(imageFD,&super,group);
    changeNumberOfDataBlocks(&number_of_data_blocks_per_group,&super,group);

    bmap inodeBitmaps [number_of_block_groups][block_size];
    bmap blockBitmaps [number_of_block_groups][block_size];
    changeInodeBitmaps(inodeBitmaps,imageFD,&super,group);
    changeBlockBitmaps(blockBitmaps,imageFD,&super,group);

    if(isTargetInode(targetPathOrTargetInode))
    {
      printf("Target is Inode\n" );
      targetInode = atoi(targetPathOrTargetInode);
      printf("Target i-node is %u \n", targetInode);
    }
    else //target is path
    {
      // printf("Target is path\n" );
      // printf("PATH IS : %s\n", targetPathOrTargetInode );
      char * currentChar = targetPathOrTargetInode;
      unsigned int seperateTokenCount = 0;
      unsigned int inName = 0;
      while(*currentChar!='\0')
      {
        // printf("Current char is %c \n", *currentChar );

        if(inName==0) // it was / now its not,,
        {
          seperateTokenCount++;
          inName = 1;
        }
        else //its in name
        {
          if(*currentChar == '/')
          {
            inName=0;
          }
        }
        currentChar++;
      }
      // printf("%u tokens found \n", seperateTokenCount);
      pathTokenizer = malloc(sizeof(char *)*seperateTokenCount);

      char * targetPathWithoutSlash = targetPathOrTargetInode;
      targetPathWithoutSlash++;
      char * currentWord;
      unsigned int windex = 0;
      //TODO zero check for root
      currentWord = strtok(targetPathWithoutSlash,"/");
      while(currentWord!=NULL)
      {
        pathTokenizer[windex] = currentWord;
        windex++;
        currentWord=strtok(NULL,"/");
      }

      //TODO now that you have the path, convert it to a inode,,,
      printf("Seperate Token Count is %u \n",seperateTokenCount );
      if(strcmp(targetPathOrTargetInode,"/")==0)
      {
        targetInode = 2;
      }
      else
      {
        targetInode = 2;
        for(unsigned int k=0;k<seperateTokenCount;k++)
        {
          char * currentDirectoryName = pathTokenizer[k];

          struct ext2_dir_entry tempReadDir;
          for(unsigned int p = 0 ; p<15;p++)
          {
            unsigned int targetBlockGroup = InodeAtWhichBlockGroup(targetInode,&super);
            unsigned int moddedTargetID = getModdedTargetInode(targetInode,&super);
            readToInode(&targetDirectoryInode,imageFD,group,targetBlockGroup,moddedTargetID);

            //TODO p<12 for directs
            unsigned int dataBlockPointer = targetDirectoryInode.i_block[p];
            unsigned int correctDirectoryFlag = 0;
            //TODO here we assume that if its not allocated its 0, and rest is 0 too... its a big if
            if(dataBlockPointer != 0)
            {
              unsigned int targetDataBlockOffset = BLOCK_OFFSET(dataBlockPointer);
              unsigned int dirEntryInode;
              unsigned short dirRecLen;
              while(1)
              {
                lseek(imageFD,targetDataBlockOffset,SEEK_SET);
                read(imageFD,&dirEntryInode,sizeof(unsigned int));
                if(dirEntryInode == 0)
                {
                  printf("Inode entry shouldn't be 0 \n");
                  //TODO temporarily break
                  break;
                }
                else
                {
                  unsigned char dirNameLen;
                  unsigned char dirFileType;
                  char * dirName;
                  read(imageFD,&dirRecLen,sizeof(unsigned short));
                  read(imageFD,&dirNameLen,sizeof(unsigned char));
                  dirName = malloc(dirNameLen+1);
                  dirName[dirNameLen]='\0';
                  read(imageFD,&dirFileType,sizeof(unsigned char));
                  read(imageFD,dirName,dirNameLen); //TODO +1 or not
                  printf("Dir: %s \n",dirName);

                  if(strcmp(dirName,currentDirectoryName)==0)
                  {
                    //TODO directory can be last direntry
                    targetInode = dirEntryInode;
                    printf("Correct Directory Found!, Next Inode is %u\n", dirEntryInode);
                    free(dirName);
                    correctDirectoryFlag=1;
                    break;
                  }

                  //TODO If this is the case update dirRecLen, write new entry under it
                  targetDataBlockOffset+=dirRecLen;
                  free(dirName);
                }
              }
              if(correctDirectoryFlag==1)
                break;
            }
          }
          //END OF LOOP
        }
      }
    }

    printf("\nMain job handler entered! \n");

    unsigned int targetBlockGroup = InodeAtWhichBlockGroup(targetInode,&super);
    unsigned int moddedTargetID = getModdedTargetInode(targetInode,&super);
    //reusing same inode structure that was used for path traversal,
    readToInode(&targetDirectoryInode,imageFD,group,targetBlockGroup,moddedTargetID);
    // printf("Target inode size is %u\n", targetDirectoryInode.i_ctime);

    struct ext2_dir_entry tempReadDir;
    for(unsigned int p = 0 ; p<15;p++)
    {
      unsigned int dataBlockPointer = targetDirectoryInode.i_block[p];
      unsigned int endOfDirectoryFlag = 0;
      //TODO here we assume that if its not allocated its 0, and rest is 0 too... its a big if
      if(dataBlockPointer != 0)
      {
        unsigned int targetDataBlockOffset = BLOCK_OFFSET(dataBlockPointer);
        unsigned int dirEntryInode;
        unsigned short dirRecLen;
        while(1)
        {
          lseek(imageFD,targetDataBlockOffset,SEEK_SET);
          read(imageFD,&dirEntryInode,sizeof(unsigned int));
          if(dirEntryInode == 0)
          {
            //end of dir entries write here;
            //TODO temporarily break
            break;
          }
          else
          {
            unsigned char dirNameLen;
            unsigned char dirFileType;
            char * dirName;
            read(imageFD,&dirRecLen,sizeof(unsigned short));
            read(imageFD,&dirNameLen,sizeof(unsigned char));
            dirName = malloc(dirNameLen);
            // dirName[dirNameLen]='\0';
            read(imageFD,&dirFileType,sizeof(unsigned char));
            read(imageFD,dirName,dirNameLen);
            printf("Name of dir entry is %s \n",dirName);

            if(((8+strlen(dirName)+3) & ~0x03) < dirRecLen) //magic :D
            {
              printf("Old dirRecLen was %u\n",dirRecLen);
              unsigned short new_rec_len = ((8+strlen(dirName)+3) & ~0x03);
              printf("Its the end of directory\n\n" );
              printf("New Record Length will be %u\n", (unsigned short) new_rec_len );
              //TODO UPDATE OLD DIRECTORY INFO, maybe superblock needs update aswell inode and block count...

              lseek(imageFD,targetDataBlockOffset+sizeof(unsigned int),SEEK_SET);
              write(imageFD,&new_rec_len,sizeof(unsigned short));

              //ALLOCATE A NEW DIRECTORY ENTRY LOCALLY
              unsigned int newLatestLen = dirRecLen-new_rec_len;
              unsigned char new_name_len = (unsigned char) strlen(sourceFilePath);
              unsigned char new_file_type = EXT2_FT_REG_FILE;



              SearchResult newFreeInode = findNextFreeInode(imageFD,&super,group);
              lseek(imageFD,targetDataBlockOffset+new_rec_len,SEEK_SET);
              write(imageFD,&newFreeInode.ID,sizeof(unsigned int));
              write(imageFD,&newLatestLen,sizeof(unsigned short));
              write(imageFD,&new_name_len,sizeof(unsigned char));
              write(imageFD,&new_file_type,sizeof(unsigned char));
              for (int i =0;i<strlen(sourceFilePath);i++) {
                write(imageFD,&sourceFilePath[i],sizeof(char));

              }
              printf("New size of last entry will be %u\n",newFreeInode.ID );
              printf("New size of last entry will be %u\n",newLatestLen );
              printf("New size of last entry will be %d\n",new_name_len );

              printf("Target inode is %u\n", newFreeInode.ID);
              printf("File record length is %u\n", (unsigned int) new_rec_len);
              printf("File name is %s \n", sourceFilePath);
              printf("File name length is %u \n\n", strlen(sourceFilePath));

              printf("Writing new entry to given path or inode\n");

              printf("Before marking---------------\n" );
              dumpGroupZeroInodeBitmap(imageFD,&super,group);
              markInodeAsSetInBitmap(imageFD,&super,group,&newFreeInode,inodeBitmaps);
              printf("After marking---------------\n" );
              dumpGroupZeroInodeBitmap(imageFD,&super,group);


              super.s_free_inodes_count--;

              group[newFreeInode.blockGroupNo-1].bg_free_inodes_count--;
              // printf("----****---- Decrements free inode count from offset %u\n",BASE_OFFSET+sizeof(struct ext2_super_block)+(sizeof(struct ext2_group_desc)*(newFreeInode.blockGroupNo-1)) );
              // lseek(imageFD, BASE_OFFSET+sizeof(struct ext2_super_block)+(sizeof(struct ext2_group_desc)*(newFreeInode.blockGroupNo-1)), SEEK_SET);
              // write(imageFD, &group[newFreeInode.blockGroupNo-1], sizeof(struct ext2_group_desc));

              free(dirName);

              //reads inode into targetDirectoryInode for easy manipulation
              readToInode(&targetDirectoryInode,imageFD,group,newFreeInode.blockGroupNo,getModdedTargetInode(newFreeInode.ID,&super));

              copyMetadata(&targetDirectoryInode,&metadataOfSource);

              printf("After Copy MetaData---------------\n" );
              dumpGroupZeroInodeBitmap(imageFD,&super,group);

              printf("-------------------------Modded %u\n",(getModdedTargetInode(newFreeInode.ID,&super)-1));

              char readBuffer [block_size] ;
              printf("File size is %ld\n",metadataOfSource.st_size );
              printf("Block size is %u\n",block_size );
              unsigned int numberOfRequiredBlocks=ceil((double)metadataOfSource.st_size/block_size);
              printf("Number of Required Blocks is %u\n\n", numberOfRequiredBlocks);

              unsigned int bytesLeftToRead = metadataOfSource.st_size;
              printf("Bytest left to read is %u\n", bytesLeftToRead);

              if(numberOfRequiredBlocks>12)
              {
                printf("Requires more than direct blocks\n");
              }
              unsigned int b = 0;
              printf("Before Reading WHİLE ---------------\n" );
              dumpGroupZeroInodeBitmap(imageFD,&super,group);
              while(bytesLeftToRead>0)
              {
                printf("In While bytes left to read is %u\n", bytesLeftToRead);
                memset(readBuffer,0x00,block_size);
                if(bytesLeftToRead>block_size)
                {
                  read(sourceFD,readBuffer,block_size);
                  bytesLeftToRead-=block_size;
                }
                else
                {
                  read(sourceFD,readBuffer,bytesLeftToRead);
                  bytesLeftToRead = 0;
                }
                printf("After reading from source---------------\n" );
                dumpGroupZeroInodeBitmap(imageFD,&super,group);

                SearchResult freeDataBlock = findNextFreeDataBlock(imageFD,&super,group);
                printf("Free data block ID is : %u \n",freeDataBlock.ID );
                // printf("Free data block at block group no : %u \n",freeDataBlock.blockGroupNo );
                printf("Before Marking Datablock---------------\n" );
                dumpGroupZeroInodeBitmap(imageFD,&super,group);
                markDataBlockAsSetInBitmap(imageFD,&super,group,&freeDataBlock,blockBitmaps);
                //TODO EXPLODES!!!
                // printf("After Marking Datablock---------------\n" );
                dumpGroupZeroInodeBitmap(imageFD,&super,group);

                super.s_free_blocks_count--;
                // lseek(imageFD, BASE_OFFSET, SEEK_SET);
                // write(imageFD, &super, sizeof(struct ext2_super_block));

                group[freeDataBlock.blockGroupNo-1].bg_free_blocks_count--;
                //TODO THIS NEEDS TO DECREASE
                printf("data block group no is %u\n",freeDataBlock.blockGroupNo-1);
                printf("target offset is %u\n",BASE_OFFSET+sizeof(struct ext2_super_block)+(sizeof(struct ext2_group_desc)*(freeDataBlock.blockGroupNo-1)));
                // lseek(imageFD, BASE_OFFSET+sizeof(struct ext2_super_block)+(sizeof(struct ext2_group_desc)*(freeDataBlock.blockGroupNo-1)), SEEK_SET);
                // write(imageFD, &group[freeDataBlock.blockGroupNo-1], sizeof(struct ext2_group_desc));

                if(b<12)
                {
                  printf("Writing to block no: %u\n", b);
                  targetDirectoryInode.i_block[b]=freeDataBlock.ID;
                  // writes data block from source to image
                  lseek(imageFD,BLOCK_OFFSET(freeDataBlock.ID),block_size);
                  write(imageFD,readBuffer,block_size);
                  //TODO change group descriptor counts
                  b++;
                }
                else if (b ==12)
                {
                  printf("Indirect point needed\n");
                  //TODO know where to put b++;
                }
                else if (b ==13)
                {
                  printf("Double point needed\n");
                  //TODO know where to put b++;
                }
                else if (b ==14)
                {
                  printf("Triple point needed\n");
                  //TODO know where to put b++;
                }

              }

              //writes back the super block
              lseek(imageFD, BASE_OFFSET, SEEK_SET);
              printf("--a-sa-d--sad-sa-d %u\n",super.s_free_inodes_count );
              printf("GROOOAJDOIWAIJDADK%u\n",group[2].bg_free_inodes_count );
              write(imageFD, &super, sizeof(struct ext2_super_block));

              //writes back group descriptor information
              lseek(imageFD,BASE_OFFSET+sizeof(struct ext2_super_block),SEEK_SET);
              write(imageFD,group,sizeof(struct ext2_group_desc)*number_of_block_groups);

              //writes back the newly allocated inode
              lseek(imageFD,BLOCK_OFFSET(group[newFreeInode.blockGroupNo-1].bg_inode_table)+((getModdedTargetInode(newFreeInode.ID,&super)-1)*sizeof(struct ext2_inode)),SEEK_SET);
              write(imageFD,&targetDirectoryInode,sizeof(struct ext2_inode));
              // TODO need to keep track of were each datablock comes from which block group

              for(unsigned int i = 0; i<number_of_block_groups;i++)
              {
                lseek(imageFD,BLOCK_OFFSET(group[i].bg_inode_bitmap),SEEK_SET);
                write(imageFD,inodeBitmaps[i],block_size);
              }
              for(unsigned int i = 0; i<number_of_block_groups;i++)
              {
                lseek(imageFD,BLOCK_OFFSET(group[i].bg_block_bitmap),SEEK_SET);
                write(imageFD,blockBitmaps[i],block_size);
              }


              endOfDirectoryFlag = 1;
              break;
            }
            //TODO If this is the case update dirRecLen, write new entry under it
            targetDataBlockOffset+=dirRecLen;
            free(dirName);
          }
        }
        if(endOfDirectoryFlag==1)
          break;
      }
    }


    printf("Before end!\n");
    dumpGroupZeroInodeBitmap(imageFD,&super,group);

    close(imageFD);
    close(sourceFD);
    printf("1Bitmap Inode Table Location %u\n", group[0].bg_inode_table);
    printf("1Bitmap Block Bitmap Location %u\n", group[0].bg_block_bitmap);
    printf("1Bitmap Inode Bitmap Location %u\n", group[0].bg_inode_bitmap);
    printf("2Bitmap Inode Table Location %u\n", group[1].bg_inode_table);
    printf("2Bitmap Block Bitmap Location %u\n", group[1].bg_block_bitmap);
    printf("2Bitmap Inode Bitmap Location %u\n", group[1].bg_inode_bitmap);
    printf("3Bitmap Inode Table Location %u\n", group[2].bg_inode_table);
    printf("3Bitmap Block Bitmap Location %u\n", group[2].bg_block_bitmap);
    printf("3Bitmap Inode Bitmap Location %u\n", group[2].bg_inode_bitmap);

    return 0;
}
