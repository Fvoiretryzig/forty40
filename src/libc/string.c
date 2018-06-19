#include<libc.h>

void *memset(void *b, int c, size_t n)
{
	if(b == NULL || n < 0){
		printf("null in memset\n");
		return NULL;
	}
	char *temp_b = (char *)b;
	while(n--)
		*temp_b++ = c;
	return b;
}
void *memcpy(void *dst, const void *src, size_t n)
{
	if(dst == NULL || src == NULL){
		printf("null in memcpy\n");
		return NULL;
	}
	char *temp_src = (char *)src;
	char *temp_dst = (char *)dst;
	if(((temp_src <= temp_dst) && (temp_dst <temp_src+n)) ||
	   ((temp_src >= temp_dst) && (temp_src <temp_src+n))){
		printf("this is overlap in memcpy");//printf("this is no solution\n");
		return NULL;
	}
    while(n--){
        *temp_dst = *temp_src;
        temp_dst++;
        temp_src++;
    }
    return dst;			
}
size_t strlen(const char* s)
{
	const char *temp = s;
	while(*temp++);
	return (temp-s-1);
}
char *strcpy(char *dst, const char *src)
{
	char *temp_dst = dst;
	//while((*temp_dst++ = *src++));
	while((*temp_dst++ = *src++)!='\0');
	printf("dst:%s\n", dst);
	printf("\n");
	return dst;
}
char *strncpy(char *dst, const char *src, size_t n)
{
	char *temp_dst = dst;
	while(n-- && (*temp_dst++ = *src++));
	if(n)
		while(--n)
			*temp_dst = '\0';
	return dst;
}
int strcmp ( const char* s1, const char* s2 )  
{  
    int ret = 0 ;  
    while(*s1 == *s2 && *s2){
    	s1++; s2++;
    }
   	int tmp = *s1 -*s2;
   	if(tmp > 0)
   		ret = 1;
   	else if(tmp < 0)
   		ret = -1;
    return ret;  
}  
int strncmp(const char *s1, const char *s2, size_t n)
{
	int ret = 0;
	while(*s1 == *s2 && *s2 && n--){
		s1++; s2++;
	}
	if(n){
		int temp = *s1 - *s2;
	    if(temp > 0)
	    	ret = 1;
	    else if(temp < 0)
	    	ret = -1; 	
	}
    return ret;  		
}
char * strcat(char * dst, const char * src)
{
	printf("in strcat: dst:%s\n", dst);
	char *tmp = dst;
	while(*dst)
		dst++;
	while ((*dst++ = *src++) != '\0');
	printf("in strcat: dst:%s\n", dst);
	printf("\n");
	return tmp;
}
char * strncat(char * front, const char * back, size_t count)
{
   char *start = front;
   while (*front++);
   while (count--){
       if (!(*front++ = *back++))
           return(start);   
   }
    *front ='\0';
   return(start);
}

