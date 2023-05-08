#include "telephone_directory.h"


struct list_head telephone_directory_head = LIST_HEAD_INIT(telephone_directory_head);

long telephone_directory_get_user_entry(const char* surname, uint32_t length, telephone_directory_t** output_entry) {
	telephone_directory_t* current_entry;

	list_for_each_entry(current_entry, &telephone_directory_head, next_elem) {
		if (strcmp(current_entry->user_data.surname, surname) == 0) {
			*output_entry = current_entry;
			return 0;
		}
	}

	return -1;
}

long telephone_directory_get_user(const char* surname, uint32_t length, user_data_t* output_data) {
	telephone_directory_t* user_entry;
	if (telephone_directory_get_user_entry(surname, length, &user_entry) == 0) {
		*output_data = user_entry->user_data;
		return 0;
	}

	return -1;
}

long telephone_directory_add_user(user_data_t* input_data) {
	telephone_directory_t* user_entry;
	if (telephone_directory_get_user_entry(input_data->surname, strlen(input_data->surname), &user_entry) == 0) {
		return -1;
	}

	telephone_directory_t* new_user = kmalloc(sizeof(telephone_directory_t), GFP_KERNEL);

	strncpy(new_user->user_data.name, input_data->name, MAX_FIELD_SIZE);
	strncpy(new_user->user_data.surname, input_data->surname, MAX_FIELD_SIZE);
	new_user->user_data.age = input_data->age;
	strncpy(new_user->user_data.telephone_number, input_data->telephone_number, MAX_FIELD_SIZE);
	strncpy(new_user->user_data.email, input_data->email, MAX_FIELD_SIZE);

	INIT_LIST_HEAD(&new_user->next_elem);
	list_add(&new_user->next_elem, &telephone_directory_head);

	return 0;
}

long telephone_directory_del_user(const char* surname, uint32_t length) {
	telephone_directory_t* user_entry;
	if (telephone_directory_get_user_entry(surname, length, &user_entry) == 0) {
		list_del(&user_entry->next_elem);
		kfree(user_entry);
		return 0;
	}

	return -1;
}

void delete_telephone_directory(void) {
	telephone_directory_t* current_entry;
	telephone_directory_t* next_entry;
	list_for_each_entry_safe(current_entry, next_entry, &telephone_directory_head, next_elem) {
		list_del(&current_entry->next_elem);
		kfree(current_entry);
	}
}
