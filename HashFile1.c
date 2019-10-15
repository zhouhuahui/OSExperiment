#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "HashFile.h"

int hashfile_creat(const char *filename,mode_t mode,int reclen,int total_rec_num)
{
    struct HashFileHeader hfh;
    int fd;
    int rtn;
    char* buf;
    int i = 0;
    hfh.sig = 31415926;
    hfh.reclen = reclen;
    hfh.total_rec_num = total_rec_num;
    hfh.current_rec_num = 0;
    fd = creat(filename ,mode);
    if(fd != -1)
    {
        rtn = write(fd ,&hfh ,sizeof(struct HashFileHeader));
        if(rtn != -1)
        {
            buf = (char*)malloc((reclen+sizeof(struct CFTag)) * total_rec_num);
            memset(buf, 0 ,(reclen+sizeof(struct CFTag))*total_rec_num);
            rtn = write(fd, buf, (reclen+sizeof(struct CFTag)) * total_rec_num);
            free(buf);
        }
        close(fd);
        return rtn;
    }
    else
    {
        close(fd);
        return -1;
    }
}


int hashfile_open(const char* filename, int flags, mode_t mode)
{
    int fd = open(filename, flags, mode);
    struct HashFileHeader hfh;
    if (read(fd, &hfh, sizeof(struct HashFileHeader)) != -1)    // 文件打开成功
    {
        lseek(fd, 0, SEEK_SET); // SEEK_SET：指向文件头，再增加offset个偏移量，0：offset
        if (hfh.sig == 31415926)    // 标志错误
            return fd;
        else
            return -1;
    } else {
        return -1;  // 打开失败
    }
}

int hashfile_close(int fd)
{
    return close(fd);
}

int hashfile_read(int fd, int keyoffset, int keylen, void* buf)
{
    struct HashFileHeader hfh;
    readHashFileHeader(fd, &hfh);
    int offset = hashfile_findrec(fd, keyoffset, keylen, buf);  // 找到读开始的位置
    if (offset != -1)
    {
        lseek(fd, offset + sizeof(struct CFTag), SEEK_SET);
        return read(fd, buf, hfh.reclen);
    } else {
        return -1;
    }
}

int hashfile_write(int fd, int keyoffset, int keylen, void* buf)
{
    return hashfile_saverec(fd, keyoffset, keylen, buf);
}

int hashfile_delrec(int fd, int keyoffset, int keylen, void* buf)
{
    int offset;
    offset = hashfile_findrec(fd, keyoffset, keylen, buf);
    if (offset != -1)
    {
        struct CFTag tag;
        read(fd, &tag, sizeof(struct CFTag));
        tag.free = 0;   // 置空闲标志
        lseek(fd, offset, SEEK_SET);
        write(fd, &tag, sizeof(struct CFTag));  // 写空闲标志
        struct HashFileHeader hfh;
        readHashFileHeader(fd, &hfh);
        int addr = hash(keyoffset, keylen, buf, hfh.total_rec_num);
        offset = sizeof(struct HashFileHeader) + addr * (hfh.reclen + sizeof(struct CFTag));
        if (lseek(fd, offset, SEEK_SET) == -1)
            return -1;
        read(fd, &tag, sizeof(struct CFTag));
        tag.collision--;    // 冲突计数减 1，并不一定是被删记录，可能是与被删记录有相同哈希值的记录
        lseek(fd, offset, SEEK_SET);
        write(fd, &tag, sizeof(struct CFTag));  // 写入，更新
        hfh.current_rec_num--;  // 删除一条记录，当前记录数减 1
        lseek(fd, 0, SEEK_SET);
        write(fd, &hfh, sizeof(struct HashFileHeader));
    }
    else
    {
        return -1;
    }
}

int hashfile_findrec(int fd,int keyoffset,int keylen,void *buf)
{
    struct HashFileHeader hfh;
    readHashFileHeader(fd, &hfh);
    int addr = hash(keyoffset, keylen, buf, hfh.total_rec_num);
    int offset = sizeof(struct HashFileHeader) + addr * (hfh.reclen + sizeof(struct CFTag));
    if(lseek(fd, offset,SEEK_SET) == -1)
        return -1;
    struct CFTag tag;
    read(fd, &tag, sizeof(struct CFTag));
    char count = tag.collision;
    if(count == 0)
        return -1; //不存在

recfree:
    if(tag.free == 0)
    {
        offset += hfh.reclen + sizeof(struct CFTag);
        if(lseek(fd, offset, SEEK_SET) == -1)
            return -1;
        read(fd, &tag, sizeof(struct CFTag));
        goto recfree;
    }
    else
    {
        char* p =(char*)malloc(hfh.reclen * sizeof(char));
        read(fd, p, hfh.reclen);
        //printf("Record is {%d , %s}\n",((struct jtRecord *)p)->key,((struct jtRecord *p)->other);
        char* p1;
        char* p2;
        p1 = (char *)buf + keyoffset;
        p2 = p + keyoffset;
        int j = 0;
        while((*p1 == *p2)&&(j < keylen))
        {
            p1++;
            p2++;
            j++;
        }
        if(j == keylen)
        {
            free(p);
            p = NULL;
            return(offset);
        }
        else
        {
            if(addr == hash(keyoffset, keylen, p, hfh.total_rec_num))
            {
                count--;
                if(count == 0)
                {
                    free(p);
                    p = NULL;
                    return -1;//不存在
                }
            }
            free(p);
            p = NULL;
            offset += hfh.reclen + sizeof(struct CFTag);
            if(lseek(fd, offset, SEEK_SET) == -1)
                return -1;
            read(fd, &tag, sizeof(struct CFTag));
            goto recfree;
        }
    }
}

int hashfile_saverec(int fd, int keyoffset, int keylen, void* buf)
{
    if (checkHashFileFull(fd))
    {
        // hfh 记录数已达最大
        return -1;
    }
    struct HashFileHeader hfh;
    readHashFileHeader(fd, &hfh);
    int addr = hash(keyoffset, keylen, buf, hfh.total_rec_num);
    int offset = sizeof(struct HashFileHeader) + addr * (hfh.reclen + sizeof(struct CFTag));
    //printf("hfh: %d\tcftag: %d\thfh.reclen: %d\taddr: %d\toffset:%d\n",sizeof(struct HashFileHeader),sizeof(struct CFTag), hfh.reclen, addr, offset);
    if (lseek(fd, offset, SEEK_SET) == -1)
        return -1;
    struct CFTag tag;
    // 先读 tag
    read(fd, &tag, sizeof(struct CFTag));
    tag.collision++;
    lseek(fd, sizeof(struct CFTag) * (-1), SEEK_CUR);
    // 从当前位置往回写入新的cftag （更新）
    write(fd, &tag, sizeof(struct CFTag));
    while (tag.free != 0)   // 冲突，顺序探查
    {
        offset += hfh.reclen + sizeof(struct CFTag);
        if (offset >= lseek(fd, 0, SEEK_END))   // 超过文件尾
            offset = sizeof(struct HashFileHeader); // 从文件头后面的第一个记录开始找
        if (lseek(fd, offset, SEEK_SET) == -1)
            return -1;
        read(fd, &tag, sizeof(struct CFTag));
    }
    // 找到 free 的 tag
    tag.free = 1;
    lseek(fd, sizeof(struct CFTag) * (-1), SEEK_CUR);
    write(fd, &tag, sizeof(struct CFTag));  // 更新 tag
    write(fd, buf, hfh.reclen); // 在 tag 之后写入实际数据
    hfh.current_rec_num++;
    lseek(fd, 0, SEEK_SET);
    return write(fd, &hfh, sizeof(struct HashFileHeader));  // 更新 hfh
}

int hash(int keyoffset, int keylen, void* buf, int recnum)
{
    int i = 0;
    char* p = (char*)buf + keyoffset;
    int addr = 0;
    for (i = 0; i < keylen; i++)
    {
        addr += (int)(*p);
        p++;
    }
    return addr % (int)(recnum * COLLISIONFACTOR);
}

int readHashFileHeader(int fd, struct HashFileHeader* hfh)
{
    lseek(fd, 0, SEEK_SET);
    return read(fd, hfh, sizeof(struct HashFileHeader));
}

int checkHashFileFull(int fd)
{
    struct HashFileHeader hfh;
    readHashFileHeader(fd, &hfh);
    if (hfh.current_rec_num < hfh.total_rec_num)
        return 0;
    else
        return 1;
}
