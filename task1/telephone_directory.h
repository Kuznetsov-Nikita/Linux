#include <linux/slab.h>
#include <linux/list.h>
#include <linux/string.h>


#define MAX_FIELD_SIZE 35


typedef struct {
	char name[MAX_FIELD_SIZE];
	char surname[MAX_FIELD_SIZE];
	uint32_t age;
	char telephone_number[MAX_FIELD_SIZE];
	char email[MAX_FIELD_SIZE];
} user_data_t;

typedef struct {
	user_data_t user_data;
	struct list_head next_elem;
} telephone_directory_t;


long telephone_directory_get_user_entry(const char* surname, uint32_t length, telephone_directory_t** output_entry);
long telephone_directory_get_user(const char* surname, uint32_t length, user_data_t* output_data);
long telephone_directory_add_user(user_data_t* input_data);
long telephone_directory_del_user(const char* surname, uint32_t length);
void delete_telephone_directory(void);
