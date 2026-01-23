//value.cpp

#include "table.h"
#include "value.h"

void Value::print() {
  switch(type) {
    case UNDEF: printf("UNDEF\n");break;
    case STRING: printf("STRING \"%s\"\n",str.c_str());break;
    case IDENTIFIER: printf("IDENTIFIER %s\n",str.c_str());break;
    case EVENTS: printf("EVENTS %ld\n",events->size());break;
    case INTEGER: printf("INTEGER %d\n",integer);break;
    case SYS_FUNCTION: printf("SYS_FUNCTION\n");break;
    case FUNCTION: printf("FUNCTION \n");break;
    case ERROR: printf("ERROR\n");break;
    case CHANNEL: printf("CHANNEL \n");break;
    case OUTPORT: printf("PORT %d\n",integer);break;
    case INPORT: printf("PORT %d\n",integer);break;
    case DICT: printf("DICT\n");dictionary->print();break;
    case LABEL: printf("LABEL :%s\n",str.c_str());break;
    case ARRAY: printf("ARRAY \n");break;
    default: printf("UNKNOWN VALUE\n");break;
  }
};
