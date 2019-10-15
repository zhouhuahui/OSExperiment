#ifdef HAVE_CONFIG_H
#include<config.h>
#endif

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include <gtk/gtk.h>
#include<ctype.h>

#include"HashFile.h"
#include"main.h"

#define KEYOFFSET 0
#define KEYLEN sizeof(int)
#define FileNAME "jing.hash"
#define TOTAL_REC_NUM 10

static GtkWidget* entry1;
static GtkWidget* text_view;
static GtkTextBuffer* buffer;

void showHashFile();
int myAtoi(const char* str);


void findRec(GtkWidget* widget, gpointer data){
	struct jtRecord jt;
	struct CFTag tag;
	const gchar* key = gtk_entry_get_text(GTK_ENTRY(entry1)); //从输入框得到键
	jt.key = myAtoi(key);
	printf("\nFind Record...\n");
	int fd = hashfile_open(FileNAME,O_RDWR,0);
	int offset = hashfile_findrec(fd,KEYOFFSET,KEYLEN,&jt);
	printf("\noffset is %d\n",offset);
	hashfile_close(fd);
	if(offset >= 0){
		int addr = hash(KEYOFFSET,KEYLEN,(void*)&jt,TOTAL_REC_NUM);
		int offset1 = sizeof(struct HashFileHeader) + addr*(RECORDLEN+sizeof(struct CFTag));
		printf("%d的哈希值是:%d\n",jt.key,addr);
		fd = open(FileNAME,O_RDWR);
		lseek(fd,offset1,SEEK_SET);
		read(fd,&tag,sizeof(struct CFTag));
		printf("Tag is <%d,%d>\t",tag.collision,tag.free);
		lseek(fd,offset+sizeof(struct CFTag),SEEK_SET);
		read(fd,&jt,sizeof(struct jtRecord));
		printf("Record is {%d,%s}\n",jt.key,jt.other);
		hashfile_close(fd);
	}
}
void readRec(){
    struct jtRecord jt;
    const gchar* key = gtk_entry_get_text(GTK_ENTRY(entry1)); //从输入框得到 键
    jt.key = myAtoi(key);

    int fd = hashfile_open(FileNAME,O_RDWR,0);
    char buf[32];
    memcpy(buf,&jt,KEYLEN);
    int flag = hashfile_read(fd,KEYOFFSET,KEYLEN,buf);
	if(flag != -1){
    	printf("\nRead Record is {%d,%s}\n",
        	((struct jtRecord *)buf)->key,((struct jtRecord *)buf)->other);
	}
	else
		printf("no record!\n");
    hashfile_close(fd);

}
void writeRec(){
    struct jtRecord jt;
	const gchar* input = gtk_entry_get_text(GTK_ENTRY(entry1)); //从输入框得到键值对
	//分割字符串
	char* key = (char*)malloc(3*sizeof(char));
	int i=0, j=0;
	while(input[i] != '\0'){
		if(input[i] != ' '){
			key[i] = input[i];
			++i;
		}
		else
			break;
	}
	++i;
	while(input[i] != '\0'){
		jt.other[j] = input[i];
		++i;
		++j;
	}
	jt.key = myAtoi(key);
	jt.other[j] = '\0';

    printf("\nWrite Record...\n");
    int fd = hashfile_open(FileNAME,O_RDWR,0);
    hashfile_saverec(fd,KEYOFFSET,KEYLEN,(void*)&jt);
    int addr = hash(KEYOFFSET,KEYLEN,(void*)&jt,TOTAL_REC_NUM);
    printf("%d的哈希值是:%d\n",jt.key,addr);
    showHashFile();
	hashfile_close(fd);
}
void deleteRec(){
    struct jtRecord jt;
	const gchar* key = gtk_entry_get_text(GTK_ENTRY(entry1)); //从输入框得到 键
	jt.key = myAtoi(key);

    printf("\nDelete Record...\n");
    int fd = hashfile_open(FileNAME,O_RDWR,0);
    hashfile_delrec(fd,KEYOFFSET,KEYLEN,&jt);
    hashfile_close(fd);
    int addr = hash(KEYOFFSET,KEYLEN,(void*)&jt,TOTAL_REC_NUM);
    printf("%d的哈希值是:%d\n",jt.key,addr);
    showHashFile();
}

int main(int argc, char* argv[]){
    gtk_init(&argc, &argv);

    GtkWidget* window;
    GtkWidget* but1;
    GtkWidget* but2;
    GtkWidget* but3;
    GtkWidget* but4;
    GtkWidget* box; //窗口
    GtkWidget* box1; //输入文本框
    GtkWidget* box2; //输出文本框
    GtkWidget* box3; //按钮

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    //设置窗体的标题
    gtk_window_set_title(GTK_WINDOW(window), "Hash File");
    //设置窗体的默认宽高
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);
    //设置窗体边框的宽度
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    box = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(window), box);
    box1 = gtk_hbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),box1,FALSE,FALSE,5);
    box2 = gtk_hbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),box2,FALSE,FALSE,200);
    GtkWidget* sep = gtk_hseparator_new();//分割线
    gtk_box_pack_start(GTK_BOX(box),sep,FALSE,FALSE,5);
    box3 = gtk_hbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),box3,TRUE,TRUE,5);

    //文本输入
    entry1 = gtk_entry_new(); //const gchar* username = gtk_entry_get_text(GTK_ENTRY(entry1));
    gtk_box_pack_start(GTK_BOX(box1),entry1,FALSE,FALSE,5);
    gtk_widget_show(entry1);

    //文本输出
    text_view = gtk_text_view_new();
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_box_pack_start(GTK_BOX(box2),text_view,FALSE,FALSE,200);
    gtk_widget_show(text_view);

    //button
    but1 =gtk_button_new_with_label("find");
    g_signal_connect(but1, "clicked", G_CALLBACK(findRec), NULL);
    gtk_box_pack_start(GTK_BOX(box3),but1,TRUE,TRUE,10);
    gtk_widget_show(but1);
    but2 =gtk_button_new_with_label("read");
    g_signal_connect(but2, "clicked", G_CALLBACK(readRec), NULL);
    gtk_box_pack_start(GTK_BOX(box3),but2,TRUE,TRUE,10);
    gtk_widget_show(but2);
    but3 =gtk_button_new_with_label("write");
    g_signal_connect(but3, "clicked", G_CALLBACK(writeRec), NULL);
    gtk_box_pack_start(GTK_BOX(box3),but3,TRUE,TRUE,10);
    gtk_widget_show(but3);
    but4 =gtk_button_new_with_label("delete");
    g_signal_connect(but4, "clicked", G_CALLBACK(deleteRec), NULL);
    gtk_box_pack_start(GTK_BOX(box3),but4,TRUE,TRUE,10);
    gtk_widget_show(but4);

    int fd = hashfile_creat(FileNAME,O_RDWR|O_CREAT,RECORDLEN,TOTAL_REC_NUM);
    int i=0;
    fd = hashfile_open(FileNAME,O_RDWR,0);
    hashfile_close(fd);
    showHashFile();

    //显示窗口
    gtk_widget_show_all(window);
    gtk_main();
	

}





int myAtoi(const char *str)//将字符串转化为数字
{
	 int num=0;
	 while(*str==' ')
	 {
		str++;
	 }
	 while(*str=='-')
	 {
		 printf("-");
		str++;
	 }
	 while(isdigit(*str))   //isdigit()是一个检查参数是否为数字字符的函数
	 {
		num=num*10+(*str-'0');//字符数字-0字符，‘1’-‘0’相当于49-48；如果写成‘\0’或0或0x00的话，就是49-0
		str++;
	 }
	 return num;
}

void showHashFile()
{
    int fd;
    printf("\n");
    fd = open(FileNAME,O_RDWR);
    lseek(fd,sizeof(struct HashFileHeader),SEEK_SET);
    struct jtRecord jt;
    struct CFTag tag;
    while(1)
    {
        if(read(fd,&tag,sizeof(struct CFTag))<=0)
            break;
        printf("Tag is <%d,%d>\t",tag.collision,tag.free);
        if(read(fd,&jt,sizeof(struct jtRecord))<=0)
            break;
        printf("Record is {%d,%s}\n",jt.key,jt.other);
    }
    close(fd);
}

