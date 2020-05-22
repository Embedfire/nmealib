#include <nmea/nmea.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <termios.h>
#include <sys/ioctl.h>

void GMTconvert(nmeaTIME *SourceTime, nmeaTIME *ConvertTime, uint8_t GMT,uint8_t AREA);

void error(const char *str, int str_size)
{
    printf("Error: ");
    write(1, str, str_size);
    printf("\n");
}

int main()
{
    int fd;
    int len;

    nmeaINFO info;
    nmeaPARSER parser;
    nmeaTIME beiJingTime;    //北京时间
    double deg_lat;//转换成[degree].[degree]格式的纬度
    double deg_lon;//转换成[degree].[degree]格式的经度

    char tmp_buf[200];
    char buff[4096];

    int size, it = 0;
    nmeaPOS dpos;

    fd = open("/dev/ttymxc2", O_RDONLY);

    printf("fopen %d\n", fd);

    if(!fd)
        return -1;

    // nmea_property()->trace_func = &trace;
    nmea_property()->error_func = &error;

    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    struct termios opt;

    //清空串口接收缓冲区
    tcflush(fd, TCIOFLUSH);
    // 获取串口参数opt
    tcgetattr(fd, &opt);

    //设置串口输出波特率
    cfsetospeed(&opt, B9600);
    //设置串口输入波特率
    cfsetispeed(&opt, B9600);
    //设置数据位数
    opt.c_cflag &= ~CSIZE;
    opt.c_cflag |= CS8;
    //校验位
    opt.c_cflag &= ~PARENB;
    opt.c_iflag &= ~INPCK;
    //设置停止位
    opt.c_cflag &= ~CSTOPB;

    //更新配置
    tcsetattr(fd, TCSANOW, &opt);

    while(1)
    {
        memset(buff, 0, 4096);
        size = 0;
        len = 0;
        nmea_zero_INFO(&info);
        nmea_parser_init(&parser);
        for(it = 0; it < 32; it++) {
            memset(tmp_buf, 0, 100);
            size = (int)read(fd, tmp_buf, 100);
            if (size > 1) {
                memcpy(buff + len, tmp_buf, size);
                len += size;
                memcpy(buff + len - 1, "\r\n", 2);
                len += 1;
            }
            
        }

        nmea_parse(&parser, buff, len, &info);

         // info.lat lon中的格式为[degree][min].[sec/60]，使用以下函数转换成[degree].[degree]格式
        deg_lat = nmea_ndeg2degree(info.lat);
        deg_lon = nmea_ndeg2degree(info.lon);

        GMTconvert(&info.utc,&beiJingTime,8,1);

        // // /* 输出解码得到的信息 */
        printf("\r\n时间：%d-%02d-%02d，%d:%d:%d\r\n", beiJingTime.year+1900, beiJingTime.mon,beiJingTime.day,beiJingTime.hour,beiJingTime.min,beiJingTime.sec);

        printf("\r\n纬度：%f,经度%f\r\n",deg_lat,deg_lon);
        printf("\r\n海拔高度：%f 米 ", info.elv);
        printf("\r\n速度：%f km/h ", info.speed);
        printf("\r\n航向：%f 度", info.direction);
        
        printf("\r\n正在使用的GPS卫星：%d,可见GPS卫星：%d",info.satinfo.inuse,info.satinfo.inview);

        printf("\r\n正在使用的北斗卫星：%d,可见北斗卫星：%d",info.satinfo.inuse,info.satinfo.inview);
        printf("\r\nPDOP：%f,HDOP：%f，VDOP：%f\n",info.PDOP,info.HDOP,info.VDOP);
    }

    nmea_parser_destroy(&parser);
    close(fd);

    return 0;
}

/******************************************************************************************************** 
**     函数名称:            bit        IsLeapYear(uint8_t    iYear) 
**    功能描述:            判断闰年(仅针对于2000以后的年份) 
**    入口参数：            iYear    两位年数 
**    出口参数:            uint8_t        1:为闰年    0:为平年 
********************************************************************************************************/ 
static uint8_t IsLeapYear(uint8_t iYear) 
{ 
    uint16_t    Year; 
    Year    =    2000+iYear; 
    if((Year&3)==0) 
    { 
        return ((Year%400==0) || (Year%100!=0)); 
    } 
     return 0; 
} 

/******************************************************************************************************** 
**     函数名称:            void    GMTconvert(uint8_t *DT,uint8_t GMT,uint8_t AREA) 
**    功能描述:            格林尼治时间换算世界各时区时间 
**    入口参数：            *DT:    表示日期时间的数组 格式 YY,MM,DD,HH,MM,SS 
**                        GMT:    时区数 
**                        AREA:    1(+)东区 W0(-)西区 
********************************************************************************************************/ 
void GMTconvert(nmeaTIME *SourceTime, nmeaTIME *ConvertTime, uint8_t GMT,uint8_t AREA) 
{ 
    uint32_t    YY,MM,DD,hh,mm,ss;        //年月日时分秒暂存变量 
     
    if(GMT==0)    return;                //如果处于0时区直接返回 
    if(GMT>12)    return;                //时区最大为12 超过则返回         

    YY    =    SourceTime->year;                //获取年 
    MM    =    SourceTime->mon;                 //获取月 
    DD    =    SourceTime->day;                 //获取日 
    hh    =    SourceTime->hour;                //获取时 
    mm    =    SourceTime->min;                 //获取分 
    ss    =    SourceTime->sec;                 //获取秒 

    if(AREA)                        //东(+)时区处理 
    { 
        if(hh+GMT<24)    hh    +=    GMT;//如果与格林尼治时间处于同一天则仅加小时即可 
        else                        //如果已经晚于格林尼治时间1天则进行日期处理 
        { 
            hh    =    hh+GMT-24;        //先得出时间 
            if(MM==1 || MM==3 || MM==5 || MM==7 || MM==8 || MM==10)    //大月份(12月单独处理) 
            { 
                if(DD<31)    DD++; 
                else 
                { 
                    DD    =    1; 
                    MM    ++; 
                } 
            } 
            else if(MM==4 || MM==6 || MM==9 || MM==11)                //小月份2月单独处理) 
            { 
                if(DD<30)    DD++; 
                else 
                { 
                    DD    =    1; 
                    MM    ++; 
                } 
            } 
            else if(MM==2)    //处理2月份 
            { 
                if((DD==29) || (DD==28 && IsLeapYear(YY)==0))        //本来是闰年且是2月29日 或者不是闰年且是2月28日 
                { 
                    DD    =    1; 
                    MM    ++; 
                } 
                else    DD++; 
            } 
            else if(MM==12)    //处理12月份 
            { 
                if(DD<31)    DD++; 
                else        //跨年最后一天 
                {               
                    DD    =    1; 
                    MM    =    1; 
                    YY    ++; 
                } 
            } 
        } 
    } 
    else 
    {     
        if(hh>=GMT)    hh    -=    GMT;    //如果与格林尼治时间处于同一天则仅减小时即可 
        else                        //如果已经早于格林尼治时间1天则进行日期处理 
        { 
            hh    =    hh+24-GMT;        //先得出时间 
            if(MM==2 || MM==4 || MM==6 || MM==8 || MM==9 || MM==11)    //上月是大月份(1月单独处理) 
            { 
                if(DD>1)    DD--; 
                else 
                { 
                    DD    =    31; 
                    MM    --; 
                } 
            } 
            else if(MM==5 || MM==7 || MM==10 || MM==12)                //上月是小月份2月单独处理) 
            { 
                if(DD>1)    DD--; 
                else 
                { 
                    DD    =    30; 
                    MM    --; 
                } 
            } 
            else if(MM==3)    //处理上个月是2月份 
            { 
                if((DD==1) && IsLeapYear(YY)==0)                    //不是闰年 
                { 
                    DD    =    28; 
                    MM    --; 
                } 
                else    DD--; 
            } 
            else if(MM==1)    //处理1月份 
            { 
                if(DD>1)    DD--; 
                else        //新年第一天 
                {               
                    DD    =    31; 
                    MM    =    12; 
                    YY    --; 
                } 
            } 
        } 
    }         

    ConvertTime->year   =    YY;                //更新年 
    ConvertTime->mon    =    MM;                //更新月 
    ConvertTime->day    =    DD;                //更新日 
    ConvertTime->hour   =    hh;                //更新时 
    ConvertTime->min    =    mm;                //更新分 
    ConvertTime->sec    =    ss;                //更新秒 
}  
