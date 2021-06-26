#include<string.h>
#include<stdlib.h>
#include<st7789vw.h>

#include<os_kernel.h>
char content[2][128];
int eval(char* order)
{
		if(order[0]==1)
		{
				lcd_display_on();
				unsigned char nowpage=order[1];
				unsigned char totalpage=order[2];
				int ptr=3;
				int i=0;
				memset(content[(unsigned)nowpage-1],0,128);
				while(ptr<=127&&order[ptr]!=0)
				{
						content[(unsigned)nowpage-1][i++]=order[ptr++];
				}
				content[(unsigned)nowpage-1][i]=0;
				return totalpage;
		}
		if(order[0]==2)
		{
		}
		if(order[0]==3)//guan bi xian shi qi
		{
				os_task_t* task =os_task_find("display");
				if(task!=OS_NULL) os_task_destroy(task);
				lcd_display_off();
		}
}
void display(void * para)
{
		char* pa=(char*) para;
		int total_page=atoi(pa);
		os_kprintf("page:%s\r\n",pa);
		os_kprintf("pageint:%d\r\n",total_page);
		os_kprintf("content:%s\r\n",content[0]);
		if(total_page==1)
		{
			  
				lcd_show_string(0, 0, 16, "%s", content[0]);
		}
		if(total_page==2)
		{
				lcd_show_string(0, 0, 16, "%s", content[0]);
				os_task_msleep(5000);
				lcd_show_string(0, 0, 16, "%s", content[1]);
		}
}
