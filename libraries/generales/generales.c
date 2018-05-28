#include "generales.h"

int min(int n1, int n2) {
	return n1 < n2 ? n1 : n2;
}

int roundNumber(double number) {
    return (number >= 0) ? (int)(number + 0.5) : (int)(number - 0.5);
}

t_link_element* list_find_element_with_param(t_list *self, void* param, bool(*condition)(void*, void*), int* index) {
	t_link_element *element = self->head;
	int position = 0;

	while (element != NULL && !condition(element->data, param)) {
		element = element->next;
		position++;
	}

	if (index != NULL) {
		*index = position;
	}

	return element;
}

void* list_find_with_param(t_list *self, void* param, bool(*condition)(void*, void*)) {
	t_link_element *element = list_find_element_with_param(self, param, condition, NULL);
	return element != NULL ? element->data : NULL;
}
