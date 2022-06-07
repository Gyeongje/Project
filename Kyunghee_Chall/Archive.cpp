#ifndef Gyeongje_archive
#define Gyeongje_archive
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>  
#include <string.h>    
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "Data.h"

PARCHIVE initialize()    
{
    PARCHIVE archive = new ARCHIVE;
    memset(archive, 0, sizeof(ARCHIVE));

    FILE* fp = fopen(ARCHIVE_NAME, "r+b");    
    if (fp == NULL)                         
    {
        fp = fopen(ARCHIVE_NAME, "w+b");      
        if (fp == NULL)                    
            return NULL;                

        archive->header.magic = 'YK';        //KY
        archive->header.version = 1;         

        if (fwrite(&archive->header, sizeof(ARCHIVE_HEADER), 1, fp) < 1)
        {
            printf("아카이브 헤더 쓰기 실패\n");
            fclose(fp);
            return NULL;
        }
    }
    else
    {
        if (fread(&archive->header, sizeof(ARCHIVE_HEADER), 1, fp) < 1)
        {
            printf("아카이브 헤더 읽기 실패\n");
            fclose(fp);
            return NULL;
        }
    }

    if (archive->header.magic != 'YK')
    {
        printf("아카이브 파일이 아닙니다.\n");
        fclose(fp);
        return NULL;
    }

    if (archive->header.version != 1)
    {
        printf("버전이 맞지 않습니다.\n");
        fclose(fp);
        return NULL;
    }

    archive->fp = fp;  

    uint32_t size = getFileSize(fp);    
    uint32_t currPos = ftell(fp);      

    while (size > currPos)        
    {
        PFILE_NODE node = new FILE_NODE;
        memset(node, 0, sizeof(FILE_NODE));

        if (fread(&node->desc, sizeof(FILE_DESC), 1, fp) < 1)
        {
            printf("아카이브 파일 읽기 실패\n");
            delete node;
            finalize(archive);
            return NULL;
        }

        node->next = archive->fileList.next;
        archive->fileList.next = node;

        currPos = ftell(fp) + node->desc.size;
        fseek(fp, currPos, SEEK_SET);
    }

    return archive;
}

void finalize(PARCHIVE archive)  
{
    PFILE_NODE curr = archive->fileList.next;  
    while (curr != NULL)
    {
        PFILE_NODE next = curr->next;
        delete curr;

        curr = next;
    }

    fclose(archive->fp);   
    delete archive;
}

bool isExist(PARCHIVE archive, string filename)
{
    PFILE_NODE curr = archive->fileList.next;   
    while (curr != NULL)
    {
        if (strcmp(curr->desc.name, filename.c_str()) == 0)
            return true;

        curr = curr->next;
    }

    return false;
}

int append(PARCHIVE archive, string filename)
{
    int ret = 0;    

    FILE* fp = fopen(filename.c_str(), "rb");
    if (fp == NULL)
    {
        cout << filename << " 파일이 없습니다.\n";
        return -1; 
    }

    uint8_t* buffer;
    uint32_t size;

    size = getFileSize(fp); 
    buffer = new uint8_t[size];
     
    if (fread(buffer, size, 1, fp) < 1)
    {
        cout << filename << " 파일 읽기 실패\n";
        delete buffer;
        fclose(fp);
        return -1;
    }


    PFILE_DESC desc = new FILE_DESC;
    memset(desc, 0, sizeof(FILE_DESC));
    strcpy(desc->name, filename.c_str());   
    desc->size = size;             

    PFILE_NODE node = archive->fileList.next;
    do {
        if (node == NULL)
        {
            fseek(archive->fp, sizeof(ARCHIVE_HEADER), SEEK_SET);
            desc->dataOffset = ftell(archive->fp) + sizeof(FILE_DESC);
        }
        else
        {
            fseek(archive->fp, node->desc.dataOffset + node->desc.size, SEEK_SET);
            desc->dataOffset = node->desc.dataOffset + node->desc.size + sizeof(FILE_DESC);
        }

        if (fwrite(desc, sizeof(FILE_DESC), 1, archive->fp) < 1)
        {
            cout << "파일 정보 쓰기 실패\n";
            ret = -1;
            break;
        }

        if (fwrite(buffer, size, 1, archive->fp) < 1)
        {
            printf("파일 데이터 쓰기 실패\n");
            ret = -1;
            break;
        }
        printf("%s 파일 추가 성공\n크기: %d\n", filename.c_str(), size);
    } while (0);
   
    delete desc;     
    delete buffer;    
    fclose(fp);      

    return ret;
}

void file_list(PARCHIVE archive)
{
    printf("파일 목록:\n");

    PFILE_NODE curr = archive->fileList.next;   
    while (curr != NULL)
    {
        printf("    %s\n", curr->desc.name);

        curr = curr->next;
    }
}

int extract(PARCHIVE archive, string filename) 
{
    PFILE_NODE curr = archive->fileList.next;
    uint32_t size = 0;
    uint8_t* buffer;

    while (curr != NULL)
    {
        if (strcmp(curr->desc.name, filename.c_str()) == 0)
        {
            int ret = 0;
            size = curr->desc.size;
            buffer = new uint8_t[size];

            fseek(archive->fp, curr->desc.dataOffset, SEEK_SET);
            do {
                if (fread(buffer, size, 1, archive->fp) < 1)
                {
                    printf("아카이브 파일 읽기 실패\n");
                    ret = -1;
                }

                FILE* fp = fopen(filename.c_str(), "wb");
                if (fp == NULL)
                {
                    printf("%s 파일 열기 실패\n", filename.c_str());
                    ret = -1;
                    break;
                }

                if (fwrite(buffer, size, 1, fp) < 1)
                {
                    printf("%s 파일 쓰기 실패\n", filename.c_str());
                    ret = -1;
                    fclose(fp);
                    break;
                }
                printf("%s 파일 추출 성공\n크기: %d\n", filename.c_str(), size);
                fclose(fp);
            } while (0);

            delete buffer;
            return ret;     
        }

        curr = curr->next;
    }
    return -1;
}

uint32_t getFileSize(FILE* fp)  
{
    uint32_t size;
    uint32_t currPos = ftell(fp);  

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);

    fseek(fp, currPos, SEEK_SET);    

    return size;
}

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        printf("filearchive <명령> <파일명>\n\n");
        printf("명령:\n");
        printf("    append   파일 추가\n");
        printf("    list     파일 목록 출력\n");
        printf("    extract  파일 추출\n");
        return 0;
    }

    PARCHIVE archive = initialize();
    if (archive == NULL)
        return -1;

    string command = argv[1];
    string filename = argv[2];

    if (command == "append")     
    {
        if (!isExist(archive, filename))      
        {
            if (append(archive, filename) == -1)
            {
                printf("%s 파일 추가 실패\n", filename.c_str());
            }
        }
        else
        {
            printf("이미 %s 파일이 추가되어 있습니다.\n", filename.c_str());
        }
    }
    else if (command == "list")  
    {
        file_list(archive);
    }
    else if (command == "extract")  
    {
        if (isExist(archive, filename))     
        {
            if (extract(archive, filename) == -1)
            {
                printf("%s 파일 추출 실패\n", filename.c_str());
            }
        }
        else
        {
            printf("%s 파일이 없습니다.\n", filename.c_str());
        }
    }

    finalize(archive);  

    return 0;
}
#endif