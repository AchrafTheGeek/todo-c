#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define MAX_DESCRIPTION_LENGTH 256
#define FILENAME "todo_list.txt"

typedef struct node {
	int id;
	char description[MAX_DESCRIPTION_LENGTH];
	bool completed;
	struct node *next;
} todo_item;

todo_item *todo_list_head = NULL;
int todo_count = 0;
int next_id = 1;

void clear_screen() {
	// portable ANSI escape sequence to clear screen
	printf("\033[2J\033[H");
}

void wait_for_enter() {
	printf("\nPress Enter to continue...");
	int c;
	// clear leftover input
	while((c = getchar()) != '\n' && c != EOF) {};
	// wait for user to press ENTER
	//getchar();
}

void list_todos() {
	clear_screen();

	if (todo_list_head == NULL) {
		printf("You dont have any todo items.\n");
	} else {
		todo_item *current = todo_list_head;
		
		while (current != NULL) {
			printf("Item #%d | %s | Done: %s\n", current->id, current->description, current->completed ? "Yes" : "No");
			current = current->next;
		}
	}

	wait_for_enter();
}


void add_task() {
	clear_screen();

	char description[MAX_DESCRIPTION_LENGTH];
	printf("Enter a todo description: ");
	if (fgets(description, MAX_DESCRIPTION_LENGTH, stdin) == NULL) {
		printf("Invalid input. Please enter a valid description.\n");
		return;
	}
	
	// remove newline and validate
	int description_length = strlen(description);

	if (description_length > 0 && description[description_length - 1] == '\n') {
		description[description_length - 1] = '\0';
		--description_length;
	}

	if (description_length == 0) {
		printf("Error: Description can not be empty.\n");
		return;
	}
	
	// allocate and append the new todo node
	todo_item *new_item = malloc(sizeof(todo_item));

	if (new_item == NULL) {
		perror("Error: Could not allocate memory for the todo.\n");
		exit(1);
	}

	new_item->next = NULL;

	// copy description and terminate the end
	strncpy(new_item->description, description, MAX_DESCRIPTION_LENGTH - 1);
	// new_item->description[MAX_DESCRIPTION_LENGTH - 1] = '\0';

	new_item->id = next_id;
	new_item->completed = false;

	todo_item **current = &todo_list_head;

	while (*current != NULL) {
		current = &(*current)->next;
	}
	*current = new_item;
	
	
	++todo_count;
	++next_id;

	printf("Success: Added a new todo item.\n");
	
	wait_for_enter();
}	

void remove_todo() {
	clear_screen();
	
	char buffer[100];
	int id;

	printf("Enter the ID of the task you want removed: ");
	if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
		if (sscanf(buffer, "%d", &id) != 1) {
			printf("Error: Enter a valid ID.\n");
			wait_for_enter();
			return;
		}
	} else {
		printf("Error: Invalid input.\n");
		wait_for_enter();
		return;
	}

	todo_item **target_node = &todo_list_head;

	while (*target_node != NULL) {
		if ((*target_node)->id == id) {
			todo_item *deleted_node = *target_node;
			*target_node = deleted_node->next;
			free(deleted_node);
			
			printf("Success: Todo item with the ID: %d has been deleted\n", id);
			wait_for_enter();
			return;
		}
		target_node = &(*target_node)->next;
	}

	printf("Error: Could not find the todo item.\n");
	wait_for_enter();
}

void mark_as_complete() {
	clear_screen();

	char buffer[100];
	int id;

	printf("Enter the ID of the task you want to mark as complete: ");
	if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
		if (sscanf(buffer, "%d", &id) != 1) {
			printf("Error: Enter a valid ID.\n");
			wait_for_enter();
			return;
		}
	} else {
		printf("Error: Invalid input.");
		wait_for_enter();
		return;
	}


	todo_item *current = todo_list_head;

	while (current != NULL) {
		if (current->id == id) {
			if (current->completed == false) {
				current->completed = true;
	
				printf("Success: Task marked as complete.\n");
			} else {
				printf("Info: Task already marked as compete.\n");
			}

			wait_for_enter();
			return;	
		}
		current = current->next;
	}
	printf("Error: Could not find the todo item.\n");
	wait_for_enter();

}

void save_tasks() {
	clear_screen();

	FILE *file = fopen(FILENAME, "w");

	if (file == NULL) {
		perror("Error: Could not open file for writing");
		wait_for_enter();
		return;
	}

	if (todo_list_head == NULL) {
		printf("Info: Your todo list is empty.\n");
		wait_for_enter();
		return;
	}

	todo_item *current = todo_list_head;
	while (current != NULL) {
		fprintf(file, "%d;%d;%s\n",
				current->id,
				current->completed? 1 : 0,
				current->description);
		current = current->next;
	}
	fclose(file);

	printf("Success: Todo list has been saved to file.\n");
	wait_for_enter();
}

void load_tasks() {
	FILE *file = fopen(FILENAME, "r");
	if (file == NULL) {
		clear_screen();
		perror("Error: Could not find and open save file.\n");
		wait_for_enter();
		return;
	}


	char buffer[512];
	int max_id = 0;
	todo_item **current = &todo_list_head;

	while (fgets(buffer, sizeof(buffer), file)) {
		int loaded_id, loaded_completed;
		char loaded_description[MAX_DESCRIPTION_LENGTH];

		if (sscanf(buffer, "%d;%d;%[^\n]", &loaded_id, &loaded_completed, loaded_description) != 3) {
			continue;
		}
		
		*current = malloc(sizeof(todo_item));
		(*current)->id = loaded_id;
		(*current)->completed = loaded_completed;
		strncpy((*current)->description, loaded_description, MAX_DESCRIPTION_LENGTH);
		(*current)->next = NULL;


		current = &(*current)->next;

		++todo_count;
		if (loaded_id > max_id) {
			max_id = loaded_id;
		}
	}
	next_id = ++max_id;

	fclose(file);
	
	clear_screen();
	list_todos();
}	

void free_todo_list() {
	todo_item *current = todo_list_head;
	todo_item *next_item;

	for (int i = 0; current != NULL; ++i) {
		next_item = current->next;
		free(current);
		current = next_item;
	}
	todo_list_head = NULL;
}

int main(void) {

	int choice;
	load_tasks();
	do {
		clear_screen();
		printf("\n--- Todo List Application ---\n");
		printf("1-Add Todo Item\n");
		printf("2-Remove Todo Item\n");
		printf("3-Mark Todo as Complete\n");
		printf("4-List Todos\n");
		printf("5-Save Todos\n");
		printf("6-Exit Application\n");
		printf("Enter your command: ");
	
		if (scanf("%d", &choice) != 1) {
			printf("Invalid input. Please enter a number.\n");
			continue;
		}

		// clean left over newline character
		while (getchar() != '\n');


		switch (choice) {
			case 1:
				add_task();
				break;
			case 2:
				remove_todo();
				break;
			case 3:
				mark_as_complete();
				break;
			case 4:
				list_todos();
				break;
			case 5:
				save_tasks();
				break;
			case 6:
				printf("Exiting application. Have a great day.\n");
				break;
			default:
				printf("Invalid command. Please try again.\n");
				break;
		}

	} while (choice != 6);
	
	free_todo_list();

	return EXIT_SUCCESS;
}
