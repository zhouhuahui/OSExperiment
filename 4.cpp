#include <gtk/gtk.h>

static GtkWidget* entry1;
static GtkWidget* text_view;
static GtkTextBuffer* buffer;

 
void print_msg(GtkWidget *widget, gpointer window) {
  g_printf("Button clicked\n");
}
 
int main(int argc, char *argv[])
{
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
	#设置窗体的标题
    gtk_window_set_title(GTK_WINDOW(window), "Hash File");
	#设置窗体的默认宽高
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
	#设置窗体边框的宽度
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	
	box = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(window), box);
	box1 = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),box1,FALSE,FALSE,5);
	box2 = gtk_hbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),box2,FALSE,FALSE,5);
	sep = gtk_hseparator_new();//分割线
	gtk_box_pack_start(GTK_BOX(box),sep,FALSE,FALSE,5);
	box3 = gtk_hbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),box3,TRUE,TRUE,5);
	
	//文本输入
	entry1 = gtk_entry_new(); //const gchar* username = gtk_entry_get_text(GTK_ENTRY(entry1));
    gtk_box_pack_start(GTK_BOX(box1),entry1,FALSE,FALSE,5);
	gtk_widget_show(entry1);
	
	//文本输出
	text_view = gtk_view_new();
	buffer = gtk_text_get_buffer(GTK_TEXT_VIEW(text_view));
	gtk_box_pack_start(GTK_BOX(box2),text_view,FALSE,FLASE,5);
	gtk_widget_show(text_view);
	
    //button
    but1 =gtk_button_new_with_label("");
    g_signal_connect(but1, "clicked", G_CALLBACK(), NULL);
	gtk_box_pack_start(GTK_BOX(box3),but1,TRUE,TRUE,10);
	gtk_widget_show(but1);
	but2 =gtk_button_new_with_label("");
    g_signal_connect(but2, "clicked", G_CALLBACK(), NULL);
	gtk_box_pack_start(GTK_BOX(box3),but2,TRUE,TRUE,10);
	gtk_widget_show(but2);
	but3 =gtk_button_new_with_label("");
    g_signal_connect(but3, "clicked", G_CALLBACK(), NULL);
	gtk_box_pack_start(GTK_BOX(box3),but3,TRUE,TRUE,10);
	gtk_widget_show(but3);
	but4 =gtk_button_new_with_label("");
    g_signal_connect(but4, "clicked", G_CALLBACK(), NULL);
	gtk_box_pack_start(GTK_BOX(box3),but4,TRUE,TRUE,10);
	gtk_widget_show(but4);
    
    gtk_container_add(GTK_CONTAINER(halign), button);
    
 
	#显示窗口
    gtk_widget_show_all(window);
    gtk_main();
	
}
