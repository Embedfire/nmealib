#include <nmea/nmea.h>
#include <string.h>
#include <stdio.h>

int main()
{
    nmeaPOS dpos;

    const char buff[] = {
        "$BDGSV,2,1,07,03,64,189,34,06,10,166,,07,52,178,34,08,53,348,17*62\r\n"
        "$BDGSV,2,2,07,10,53,339,,13,51,304,19,16,14,168,12*56\r\n"
        "$GNRMC,084846.000,A,2253.72292,N,11350.70290,E,0.00,285.74,210520,,,A*7E\r\n"
        "$GNVTG,285.74,T,,M,0.00,N,0.00,K,A*2F\r\n"
        "$GNZDA,084846.000,21,05,2020,00,00*48\r\n"
        "$GPTXT,01,01,01,ANTENNA OK*35\r\n"
        "$GNGGA,084847.000,2253.72287,N,11350.70289,E,1,16,0.8,85.1,M,0.0,M,,*4A\r\n"
        "$GNGLL,2253.72287,N,11350.70289,E,084847.000,A,A*4C\r\n"
        "$GPGSA,A,3,02,04,06,09,17,19,28,33,34,35,36,,1.5,0.8,1.2*34\r\n"
        "$BDGSA,A,3,03,07,08,13,16,,,,,,,,1.5,0.8,1.2*25\r\n"
        "$GPGSV,4,1,14,02,32,285,23,03,07,037,,04,13,078,16,05,15,213,*7B\r\n"
        "$GPGSV,4,2,14,06,51,323,18,09,23,112,21,12,06,323,,17,57,049,30*77\r\n"
        "$GPGSV,4,3,14,19,55,009,20,28,47,172,40,33,53,109,28,34,26,169,19*70\r\n"
        "$GPGSV,4,4,14,35,61,070,38,36,59,149,36*71\r\n"
        "$BDGSV,2,1,07,03,64,189,34,06,10,166,,07,52,178,34,08,53,348,17*62\r\n"
        "$BDGSV,2,2,07,10,53,339,,13,51,304,19,16,14,168,12*56\r\n"
    };


    int it;
    nmeaINFO info;
    nmeaPARSER parser;

    nmeaTIME beiJingTime;    //北京时间
    double deg_lat;//转换成[degree].[degree]格式的纬度
    double deg_lon;//转换成[degree].[degree]格式的经度

    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    printf("%s", buff);
    nmea_parse(&parser, buff, strlen(buff), &info);

    // info.lat lon中的格式为[degree][min].[sec/60]，使用以下函数转换成[degree].[degree]格式
    deg_lat = nmea_ndeg2degree(info.lat);
    deg_lon = nmea_ndeg2degree(info.lon);
    
    printf("\r\n时间%d-%02d-%02d,%d:%d:%d\r\n", beiJingTime.year+1900, beiJingTime.mon,beiJingTime.day,beiJingTime.hour,beiJingTime.min,beiJingTime.sec);

    printf("\r\n纬度：%f,经度%f\r\n",deg_lat,deg_lon);
    printf("\r\n海拔高度：%f 米 ", info.elv);
    printf("\r\n速度：%f km/h ", info.speed);
    printf("\r\n航向：%f 度", info.direction);
    
    printf("\r\n正在使用的GPS卫星：%d,可见GPS卫星：%d",info.satinfo.inuse,info.satinfo.inview);

    printf("\r\n正在使用的北斗卫星：%d,可见北斗卫星：%d",info.satinfo.inuse,info.satinfo.inview);
    printf("\r\nPDOP：%f,HDOP：%f，VDOP：%f",info.PDOP,info.HDOP,info.VDOP); 


    nmea_parser_destroy(&parser);

    return 0;
}


