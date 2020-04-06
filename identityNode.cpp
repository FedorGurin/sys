#include "identityNode.h"
#include <string>
#include <vector>

#ifdef VXWORKS_PLATFORM
#include <net/utils/ifconfig.h> 
#include <net/if.h>
#include "sockLib.h"
#include "inetLib.h" 
#include "ioLib.h" 
#include <sys/ioctl.h>
#endif

#include <stdlib.h>
#include <iostream>
#include "globalNameID.h"

#include "dispCV1.h"
//#include "dispPV1.h"
#include "dispPV2.h"
#include "dispPV3.h"
#include "dispPV4.h"

typedef struct TTableNode_
{
    std::string nameId;
    std::string mac;
    std::string ip;    
}TTableNode;
IndentityNode::IndentityNode(uint32_t uid):ICalculateElement(uid)
{    
    std::string idStr = getNodeID();
       idNode = ID_NODE_DEFAULT;
       if(idStr == "CV1")
           idNode = ID_NODE_CV_1;
       if(idStr == "CV2")
           idNode = ID_NODE_CV_2;       
       if(idStr == "PV1")
           idNode = ID_NODE_PV_1;
       if(idStr == "PV2")
           idNode = ID_NODE_PV_2;
       if(idStr == "PV3")
          idNode = ID_NODE_PV_3;
       if(idStr == "PV4")
           idNode = ID_NODE_PV_4;
       if(idStr == "RP")
           idNode = ID_NODE_PV_RP;
       
       
       switch(idNode)
       {
           case ID_NODE_CV_1:{addElement(new DispatcherCV1(ID_ProtoCV_1));break;}
           //case ID_NODE_PV_1:{addElement(new DispatcherCV1(ID_ProtoPV_1));break;}
           case ID_NODE_PV_2:{addElement(new DispatcherPV2(ID_ProtoPV_2));break;}
           case ID_NODE_PV_3:{addElement(new DispatcherPV3(ID_ProtoPV_3));break;}
           case ID_NODE_PV_4:{addElement(new DispatcherPV4(ID_ProtoPV_4));break;}   
       };
       setStop();           
}

void IndentityNode::init()
{
   
}
std::string IndentityNode::getNodeID(void)
{
#ifdef VXWORKS_PLATFORM
    std::vector<TTableNode> listNode;
    std::string curMac;
    struct ifreq ifr;
    int fd;
    fd = socket(AF_INET,SOCK_DGRAM,0);
    if(fd == ERROR)
    {
        std::cout<<"getNodeID(): Didn`t open socket"<<std::endl;
        return "";
    }
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name,"eth0",IFNAMSIZ-1);
    int flag = ioctl(fd,SIOCGIFLLADDR,&ifr);
    if(flag == ERROR)
    {
        std::cout<<"getNodeID(): Can`t get MAC addres for eth0"<<std::endl;
        return "";
    }
    unsigned char* mac = (unsigned char*)ifr.ifr_addr.sa_data;
    
    char mas[2];
    for(int i=0;i<6;i++)
    {
        sprintf(mas,"%02X",mac[i]);
        curMac+=mas;
        if(i<5)
            curMac+="-";
    }   
    
    
    //printf("%02X:%02X:02X\n",mac[0],mac[1],mac[2]);
    //! далее нужно пробежаться по файлу и найти соотвествующий ему ip
    int fdMac = open("/romfs/mac_addr.txt",O_RDONLY,0644);
    if(fdMac == ERROR)
    {
        std::cout<<"getNodeID(): Can`t open file - mac_addr.txt"<<std::endl;
        return "";     
    }
   
    int sizeFile =  lseek(fdMac,0,SEEK_END);
    lseek(fdMac,0,SEEK_SET);
    char *buffer = new char[sizeFile+1];
    int bytes = read (fdMac,buffer,sizeFile);
    std::string dataFile;
    dataFile.append(buffer);
    //printf("%s\n",dataFile.c_str());
    
    std::string::size_type n;
    n = dataFile.find(";");
   
    //! убираем все пробельные символы
    n = dataFile.find(" ");
    while(n!=std::string::npos)
    {      
        dataFile.erase(n,1);     
        n = dataFile.find(" ");
    };
    //! убираем переходы на новую строчку
    n = dataFile.find("\r\n");
    while(n!=std::string::npos)
    {      
        dataFile.erase(n,2);  
        n = dataFile.find("\r\n");
    };
    
    n = dataFile.find(";");    
    TTableNode tempNode;
    while(n!=std::string::npos)
    {
        
        for(int j = 0;j<3;j++)
        {            
            std::string subStr = dataFile.substr(0,n);
            dataFile.erase(0,n + 1);
             
            n = dataFile.find(";");
            //i += subStr.size();
            
            switch(j)
            {
            case 0:{tempNode.nameId = subStr;break;}
            case 1:{tempNode.mac = subStr;break;}
            case 2:{tempNode.ip = subStr;break;}            
            };
          
        }   
        listNode.push_back(tempNode);    
       
    };
    //! ищем данные
    std::string comIfConfig = "eth0 ";
    
    for(int i =0;i<listNode.size();i++)
    {
        if(listNode[i].mac == curMac)
        {
            //! далее заменить ip на считанный из файла
            comIfConfig+=listNode[i].ip;
            ifconfig((char*)(comIfConfig.c_str()));  
            close(fd);
            return listNode[i].nameId;
            
            
        }
    }    
    close(fd);
    #endif
    return 0;

}
