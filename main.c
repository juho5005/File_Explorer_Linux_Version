#define TRUE 1
#define FALSE 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <semaphore.h>
#include <wait.h>

/// File, Directory, LCRS, Stack's struct
// 1) In linux, file's struct
typedef struct File {
	char name[50]; // file's name
	char content[1024]; // file's contents
	char viewtype; // file's viewtype
	int chmod__; // file's authority

	struct tm* t;
	struct File* Rsibling;
}file;

// 2) In linux, Directory's struct
typedef struct Lcrsnode {
	char name[50]; // Directory's name
	char viewtype; // Directory's viewtype
	int chmod__; // Directory's authority

	struct tm* t; // Directory's Creation time
	struct Lcrsnode* Pptr; // Upper Directory
	struct Lcrsnode* Lchild; // Lower Directory
	struct Lcrsnode* Rsibling; // Next DIrectory

	file* infile; // In Directory's file ( LCRS's next nodes)
}directory;

// 3) Implementation LCRS__Structure
typedef struct Lcrsnodestructure {
	directory* Hnode; // Upper node
	directory* Cnode; // Current node
}tree;

// 4) Impementation Stack__Structure
typedef struct stack {
	char* strname[2000]; // string's name
	int top; // Initial top = -1
}stack;



//// file, directory, tree's initial setup
// 1) Initial setup with file
file* Initial__File__Creation(char* __fileName) {
	time_t timer; // Set timer
	timer = time(NULL); // representation date

	struct tm* temptime = (struct tm*)malloc(sizeof(struct tm));
	file* temp = (file*)malloc(sizeof(file));

	if (temp != NULL) { // when temp is not NULL
		temp->chmod__ = 755; // initialized chmod's mode 755
		strncpy(temp->name, __fileName, 20); // Copying file name

		temp->Rsibling = NULL; // File's Right sibling is NULL;

		if (__fileName[0] == '.')  // make file to hide in screen
			temp->viewtype = 'u'; // view mode = 'u' mode
		else
			temp->viewtype = 's'; // view mode = 's' mode
		memcpy(temptime, localtime(&timer), sizeof(struct tm));
		temp->t = temptime;
		return temp;
	}
}

// 2) Initial setup with tree
tree* Initial__Tree__Creation(directory* root) {
	tree* temp = (tree*)malloc(sizeof(tree)); // Allocation in temp
	if (temp != NULL) {
		temp->Hnode = root; // Initialized header node NULL
		temp->Cnode = root; // Initialized child  node NULL
		return temp; // Return Directory
	}
}
tree* structure;

// 3) Initial setup with directory
directory* Initial__Directory__Creation() {
	time_t timer; // Set timer
	struct tm* temptime = (struct tm*)malloc(sizeof(struct tm));
	timer = time(NULL); // representation date

	directory* temp = (directory*)malloc(sizeof(directory));
	if (temp != NULL) {
		temp->Lchild = NULL; // leftchild's NULL
		temp->Rsibling = NULL; // rightchild's NULL
		temp->Pptr = NULL; // Parent's NULL

		strncpy(temp->name, "/", 20);

		temp->chmod__ = 755; // initialized chmod's mode 755
		temp->infile = NULL; // initialized infile NULL
		temp->viewtype = 's'; // view mode = 's' mode

		// copying memory
		memcpy(temptime, localtime(&timer), sizeof(struct tm));
		temp->t = temptime;
		return temp;
	}
}
directory* Leave__Directory(tree* dtree, char* dirname);

//// Making command (Init__Stack, Is__empty, Push, Pop)
// 1) Init__Stack
void Init__Stack(stack* pstack) {
	pstack->top = -1;
}
stack* Stack_Create()
{
	stack* pstack = (stack*)malloc(sizeof(stack));
	Init__Stack(pstack);
	return pstack;
}
stack* qstack;

// 2) Is__Empty
int Is__Empty(stack* pstack) {
	if (pstack->top == -1)
		return TRUE;
	else
		return FALSE;
}

// 3) Push
void Push(stack* pstack, char* str) {
	pstack->top += 1;
	pstack->strname[pstack->top] = str;
}

void Pop(stack* pstack) {
	if (Is__Empty(pstack) == TRUE)
		return;
	else
		pstack->top -= 1;
}

// set global pointer
sem_t semp;


// ------------------------------ main ----------------------------------
int main()
{
	// Make Root with Directory, Tree, Stack
	directory* root = Initial__Directory__Creation();
	structure = Initial__Tree__Creation(root);
	qstack = Stack_Create();

	char* look__file; // look file
	int len;
	char command[100]; // command
	char Buffer[100]; // buffer

	// "recording.txt" is able to record past command's
	// get a role with saving previous file, directory
	// Invoke already created file "recording.txt"
	FILE* fp = fopen("saving.txt", "a+");
	if (fp != NULL) {
		while (1) {
			look__file = fgets(Buffer, sizeof(Buffer), fp);
			if (feof(fp))
				break;

			// calculate file's length
			len = strlen(look__file);

			// Allocation command's str
			char* command__str = (char*)malloc(len + 1);
			strncpy(command__str, look__file, len - 1);

			*(command__str + len - 1) = '\0';
			User__Command(command__str, 0);

		}
	}
	// close file
	fclose(fp);


	while (1)
	{
		printf("OS__team2 : "); // First defaulted name
		Function__pwd(structure, qstack, 1); // First position

		// get command
		gets(command);

		// carry out command with command number
		User__Command(command, 1);
	}
}


// Making new Directory ( node )
directory* Initial__Directory(char* str)
{
	time_t timer;
	timer = time(NULL); // representation of date

	struct tm* time__tmp = (struct tm*)malloc(sizeof(struct tm));
	directory* New__Directory = (directory*)malloc(sizeof(directory));

	New__Directory->Lchild = NULL; // new Directory's lchild = NULL
	New__Directory->Rsibling = NULL; // new Directory's rsibling = NULL
	New__Directory->Pptr = NULL; // new Directory's Parent = NULL

	strncpy(New__Directory->name, str, 20);

	New__Directory->chmod__ = 755; // initialized chmod's mode 755
	New__Directory->infile = NULL; // Directory's inside is NULL

	if (str[0] == '.')
		New__Directory->viewtype = 'u';
	else
		New__Directory->viewtype = 's';
	// copying memory
	memcpy(time__tmp, localtime(&timer), sizeof(struct tm));

	New__Directory->t = time__tmp;

	return New__Directory;
}


// when creating lower directory
// connect upper directory's leftchild with new directory
void Connection__Directory(directory* parent, directory* child) {
	directory* temp = NULL;

	// if lefchild is NULL, connect this position
	if (parent->Lchild == NULL) {
		parent->Lchild = child;
		child->Pptr = parent;
	}
	// if leftchild is full, connect directory's rsibling
	else {
		temp = parent->Lchild;
		while (temp->Rsibling != NULL)
		{
			temp = temp->Rsibling;
		}

		temp->Rsibling = child;
		child->Pptr = parent;
	}
}


//// Command's Muster
// Command 1) mkdir
directory* Function__mkdir(tree* dtree, char* str, int option) {
	if (strchr(str, " ") == NULL) {
		if (Leave__Directory(dtree, str) != NULL) {
			printf("Same Directory name is already existed!!\n");
			return;
		}
	}
	if (option == 0) {
		directory* NewDir = Initial__Directory(str);
		directory* Parent = dtree->Cnode;
		Connection__Directory(Parent, NewDir);
	}
	else if (option == 1) { // Command = "mkdir -m" (give privilege)
		directory* NewDir = Initial__Directory(str);
		directory* parent = dtree->Cnode;
		Connection__Directory(parent, NewDir);
		return NewDir;
	}
	else {
		// Command = "mkdir -p"
		// ex) mkdir -p abc/def/ghi
		directory* NewDir = Initial__Directory(str);
		directory* parent = dtree->Cnode;
		Connection__Directory(parent, NewDir);
	}
}

// Command 2) pwd
void Function__pwd(tree* dtree, stack* tempstack, int type) {
	directory* present = dtree->Cnode;
	if (present->Pptr == NULL) {
		printf("/");
		if (type == 0)
			printf("\n"); // In case, root
		return;
	}
	// Put every Directory in Stack
	while (present->Pptr != NULL) {
		Push(tempstack, present->name);
		present = present->Pptr;
	}
	printf("/");

	// Until Stack is empty, print every Directorys
	while (1) {
		if (Is__Empty(tempstack) == TRUE)
			break;
		else {
			printf("%s/", tempstack->strname[tempstack->top]);
			Pop(tempstack);
		}
	}
	if (type == 0)
		printf("\n");
}

// Out Directory
directory* Leave__Directory(tree* dtree, char* dirname) {
	directory* tmp = NULL;
	tmp = dtree->Cnode->Lchild;
	while (tmp != NULL) {
		if (strcmp(tmp->name, dirname) == 0)
			break;
		tmp = tmp->Rsibling;
	}
	return tmp;
}

// Out File
file* Leave__File(tree* dtree, char* filename) {
	file* tempfile = dtree->Cnode->infile;
	while (tempfile != NULL) {
		if (strcmp(tempfile->name, filename) == 0)
			break;
		tempfile = tempfile->Rsibling;
	}
	return tempfile;
}


// Command 3) cd
char* cd__utill(char* path) {
	int len = strlen(path);
	char* temp = (char*)malloc(len);
	strncpy(temp, path + 1, len);
	*(temp + len - 2) = '\0';
	return temp;
}

int Function__cd(tree* dtree, char* path) {
	if (strcmp(path, ".") == 0) // '.' : Current Directory
		return TRUE;
	else if (strcmp(path, "..") == 0) {
		if (dtree->Cnode == dtree->Hnode)
			return TRUE;
		else {
			dtree->Cnode = dtree->Cnode->Pptr;
			return TRUE;
		}
	}
	else {
		if (Leave__Directory(dtree, path) == NULL)
			return FALSE;
		else {
			dtree->Cnode = Leave__Directory(dtree, path);
			return TRUE;
		}
	}
}

void Change__Directory__position(tree* dtree, char* path) {
	char* str = NULL;
	char temp[200];
	directory* nodetemp;

	strncpy(temp, path, 100);

	nodetemp = dtree->Cnode;
	if (strcmp(path, "/") == 0)
		dtree->Cnode = dtree->Hnode;

	else if (path[0] == '/') {
		path = cd__utill(path);
		str = strtok(path, "/");
		dtree->Cnode = dtree->Hnode;
		while (str != NULL) {
			if (Function__cd(dtree, str) == FALSE)
				break;
			str = strtok(NULL, "/");
		}
	}
	else
		Function__cd(dtree, path);
}

// Command 4) cat
// Initialize cat
void Initial__cat(tree* dtree, char* filename, char* cat_content) {
	int len;
	file* temp__file = Initial__File__Creation(filename);
	file* current__file = dtree->Cnode->infile;

	if (current__file == NULL) {
		dtree->Cnode->infile = temp__file;
		strncpy(dtree->Cnode->infile->content, cat_content, 1024);
	}
	else {
		if (current__file->Rsibling == NULL) {
			current__file->Rsibling = temp__file;
			strncpy(dtree->Cnode->infile->Rsibling->content, cat_content, 1024);
		}
		else {
			current__file = current__file->Rsibling;

			while (current__file->Rsibling != NULL)
				current__file = current__file->Rsibling;

			current__file->Rsibling = temp__file;
			strncpy(temp__file->content, cat_content, 1024);
		}
	}
}
// Implementation cat
void Function__cat(tree* dtree, char* filename, int option) {
	directory* tempnode = dtree->Cnode;
	file* temp__file = dtree->Cnode->infile;
	file* current__file = NULL;

	if (option == 0) {
		if (temp__file == NULL)
			printf("File or Directory No exist\n");
		else {
			if (strcmp(temp__file->name, filename) == 0)
				printf("%s", temp__file->content);
			else {
				while (temp__file->Rsibling != NULL) {
					temp__file = temp__file->Rsibling;
					if (strcmp(temp__file->name, filename) == 0) {
						printf("%s", temp__file->content);
						return;
					}
				}
				printf("File or Directory No exist\n");
			}
		}
	}

	// option 1 ==> cat '>'
	else if (option == 1) {
		if (temp__file == NULL) {
			char temp[1024];
			temp__file = Initial__File__Creation(filename);
			fgets(temp, 1024, stdin);

			strncpy(temp__file->content, temp, 1024);

			tempnode->infile = temp__file;
		}
		else {
			if (temp__file->Rsibling == NULL) {
				char temp[1024];
				temp__file->Rsibling = Initial__File__Creation(filename);
				fgets(temp, 1024, stdin);
				strncpy(temp__file->Rsibling->content, temp, 1024);
			}
			else {
				current__file = temp__file->Rsibling;
				while (current__file->Rsibling != NULL)
				{
					current__file = current__file->Rsibling;
				}
				char temp[1024];
				temp__file = Initial__File__Creation(filename);
				fgets(temp, 1024, stdin);
				strncpy(temp__file->content, temp, 1024);
				current__file->Rsibling = temp__file;
			}
		}

	}
}


// Command 5) ls
void Function__ls(tree* dtree, int option) {
	int i = 0;
	directory* current = dtree->Cnode;
	directory* tmp;
	directory* Dir__Disk[40];
	if (current->Lchild == NULL) {

	}
	else {
		tmp = current->Lchild->Rsibling;
		if (tmp == NULL)
			Dir__Disk[i++] = current->Lchild;
		else {
			Dir__Disk[i++] = current->Lchild;
			while (tmp != NULL) {
				Dir__Disk[i++] = tmp;
				tmp = tmp->Rsibling;
			}
		}
	}

	if (option == 0) {
		int num = 0;
		while (num < i) {
			if (Dir__Disk[num]->viewtype == 's')
				printf("%s ", Dir__Disk[num]->name);
			num++;
		}
		if (current->infile != NULL)
			PrintFile(structure, option);
		printf("\n");
	}
	else if (option == 1) { // ls-a
		int num = 0;
		while (num < i) {
			struct tm* dirtime = Dir__Disk[num]->t;
			if (Dir__Disk[num]->viewtype == 's') {
				printf("d");
				Authority__Directory(Dir__Disk[num]);
				Number__LCRS__Inside(Dir__Disk[num]);
				printf("4096  ");
				printf("%02d-%02d %02d:%02d:%02d    ", dirtime->tm_mon + 1, dirtime->tm_mday, dirtime->tm_hour - 3, dirtime->tm_min, dirtime->tm_sec);
				printf("%s\n", Dir__Disk[num]->name);
			}
			num++;
		}
		if (current->infile != NULL) {
			PrintFile(structure, option);
		}
		printf("\n");
	}
	else if (option == 2) { // ls-l
		int num = 0;
		while (num < i) {
			printf("%s ", Dir__Disk[num]->name);
			num++;
		}
		if (current->infile != NULL)
			PrintFile(structure, option);
		printf("\n");
	}
	else {
		int num = 0;
		while (num < i) {
			struct tm* dirtime = Dir__Disk[num]->t;
			printf("d");
			Authority__Directory(Dir__Disk[num]);
			Number__LCRS__Inside(Dir__Disk[num]);
			printf("4096  ");
			printf("%02d-%02d %02d:%02d:%02d    ", dirtime->tm_mon + 1, dirtime->tm_mday, dirtime->tm_hour-3, dirtime->tm_min, dirtime->tm_sec);
			printf("%s\n", Dir__Disk[num]->name);
			num++;
		}
		if (current->infile != NULL)
			PrintFile(structure, option);
		printf("\n");
	}
}

// representation of Tree (Preorder Tree)
int preorder__LCRS(directory* dir, int num) {
	if (dir != NULL) {
		num++;
		if (dir->infile == NULL) {
			num = preorder__LCRS(dir->Lchild, num);
			num = preorder__LCRS(dir->Rsibling, num);
		}
		else {
			file* temp = dir->infile;
			while (temp != NULL) {
				num++;
				temp = temp->Rsibling;
			}
			num = preorder__LCRS(dir->Lchild, num);
			num = preorder__LCRS(dir->Rsibling, num);
		}
	}
	return num;
}

// return preorder__LCRS's value
// minus number of searching temp node's level node
int Number__LCRS__Inside(directory* dir) {
	int num = 0;
	num = preorder__LCRS(dir, 0);
	if (dir->Rsibling != NULL) {
		directory* temp = dir->Rsibling;
		while (temp != NULL) {
			num--;
			if (temp->Lchild != NULL) {
				directory* temp1 = temp->Lchild;
				while (temp1 != NULL) {
					num--;
					temp1 = temp1->Lchild;
				}
			}
			temp = temp->Rsibling;
		}
	}
	printf("%d", num);
	printf("   ");
}

// Authority__Directory
// get Directory's authority from number and show each position
int Authority__Directory(directory* dir) {
	int chm = dir->chmod__;
	int i = 0;
	int k = 100;
	int num;

	while (i < 3) {
		num = chm / k;
		chm = chm - num * k;
		k = k / 10;
		switch (num) {
		case 0:
			printf("---");
			break;
		case 1:
			printf("--x");
			break;
		case 2:
			printf("-w-");
			break;
		case 3:
			printf("-wx");
			break;
		case 4:
			printf("r--");
			break;
		case 5:
			printf("r-x");
			break;
		case 6:
			printf("rw-");
			break;
		case 7:
			printf("rwx");
			break;
		}
		i++;
	}


	printf(" ");
}


// Authority__File
// get file's authority from number and show each position
void Authority__File(file* fi) {
	int chm = fi->chmod__;
	int k = 100;
	int i = 0;
	int num;
	while (i < 3)
	{
		num = chm / k;
		chm = chm - num * k;
		k = k / 10;
		switch (num)
		{
		case 0:
			printf("---");
			break;
		case 1:
			printf("--x");
			break;
		case 2:
			printf("-w-");
			break;
		case 3:
			printf("-wx");
			break;
		case 4:
			printf("r--");
			break;
		case 5:
			printf("r-x");
			break;
		case 6:
			printf("rw-");
			break;
		case 7:
			printf("rwx");
			break;
		}
		i++;
	}
	printf(" ");
}

// Print__File
int PrintFile(tree* dtree, int option) {
	file* temp__file = dtree->Cnode->infile;
	if (option == 0) {
		if (temp__file->viewtype == 's')
			printf("%s  ", temp__file->name);
		while (temp__file->Rsibling != NULL) {
			temp__file = temp__file->Rsibling;
			if (temp__file->viewtype == 's')
				printf("%s ", temp__file->name);
		}
	}

	else if (option == 1) {
		struct tm* filetime = temp__file->t;
		printf("-");
		Authority__File(temp__file);

		if (temp__file->viewtype == 's') {
			printf("1   "); // file's inside
			printf("%d  ", sizeof(temp__file->content));
			printf("%02d-%02d %02d:%02d:%02d    ", filetime->tm_mon + 1, filetime->tm_mday, filetime->tm_hour-3, filetime->tm_min, filetime->tm_sec);
			printf("%s\n", temp__file->name);
		}

		while (temp__file->Rsibling != NULL) {
			temp__file = temp__file->Rsibling;
			filetime = temp__file->t;
			if (temp__file->viewtype == 's') {
				printf("-");
				Authority__File(temp__file);
				printf("1   ");
				printf("%d  ", sizeof(temp__file->content));
				printf("%02d-%02d %02d:%02d:%02d    ", filetime->tm_mon + 1, filetime->tm_mday, filetime->tm_hour-3, filetime->tm_min, filetime->tm_sec);
				printf("%s\n", temp__file->name);
			}
		}
	}
	else if (option == 2) {
		printf("%s ", temp__file->name);
		while (temp__file->Rsibling != NULL)
		{
			temp__file = temp__file->Rsibling;
			printf("%s ", temp__file->name);
		}
	}

	else { // option == 3
		struct tm* filetime = temp__file->t;
		printf("-");
		Authority__File(temp__file);
		printf("1   ");  //file inside
		printf("%d  ", sizeof(temp__file->content));
		printf("%02d-%02d %02d:%02d:%02d    ", filetime->tm_mon + 1, filetime->tm_mday, filetime->tm_hour-3, filetime->tm_min, filetime->tm_sec);
		printf("%s\n", temp__file->name);
		while (temp__file->Rsibling != NULL) {
			temp__file = temp__file->Rsibling;
			filetime = temp__file->t;
			printf("-");
			Authority__File(temp__file);
			printf("1   ");
			printf("%d  ", sizeof(temp__file->content));
			printf("%02d-%02d %02d:%02d:%02d    ", filetime->tm_mon + 1, filetime->tm_mday, filetime->tm_hour-3, filetime->tm_min, filetime->tm_sec);
			printf("%s\n", temp__file->name);
		}
	}
}

// Command 6) cp
void Function__cp(tree* dtree, char* filename, char* ForDname) {
	directory* current__dir = dtree->Cnode;
	file* copy__file;
	file* temp__file;
	file* search__file;
	if (Leave__File(dtree, filename) == NULL) {
		printf("error exists!! : That file doesn't exist\n");
		return 0;
	}
	else
		copy__file = Leave__File(dtree, filename);

	if (strchr(ForDname, '/') == NULL) {
		// file ->(copy) -> file
		if (Leave__Directory(dtree, ForDname) == NULL) {
			temp__file = Initial__File__Creation(ForDname);

			strncpy(temp__file->content, copy__file->content, 1024);
			search__file = current__dir->infile->Rsibling;

			if (search__file == NULL)
				current__dir->infile->Rsibling = temp__file;
			else {
				while (search__file->Rsibling != NULL) {
					search__file = search__file->Rsibling;
				}
				search__file->Rsibling = temp__file;
			}
		}

		else {
			Change__Directory__position(dtree, ForDname);
			temp__file = dtree->Cnode->infile;

			if (temp__file == NULL) {
				temp__file = Initial__File__Creation(filename);
				strncpy(temp__file->content, copy__file->content, 1024);
				dtree->Cnode->infile = temp__file;
			}
			else {
				if (temp__file->Rsibling == NULL) {
					temp__file->Rsibling = Initial__File__Creation(filename);
					strncpy(temp__file->Rsibling->content, copy__file->content, 1024);
				}
				else {
					search__file = temp__file->Rsibling;
					while (search__file->Rsibling != NULL)
						search__file = search__file->Rsibling;
					temp__file = Initial__File__Creation(filename);
					strncpy(temp__file->content, copy__file->content, 1024);
					search__file->Rsibling = temp__file;
				}
			}
			dtree->Cnode = current__dir;
		}
	}
	else {
		ForDname = strtok(ForDname, "/");
		Change__Directory__position(dtree, ForDname);
		temp__file = dtree->Cnode->infile;
		ForDname = strtok(NULL, "/");

		if (temp__file == NULL) {
			temp__file = Initial__File__Creation(ForDname);
			strncpy(temp__file->content, copy__file->content, 1024);
			dtree->Cnode->infile = temp__file;
		}
		else {
			if (temp__file->Rsibling == NULL) {
				temp__file->Rsibling = Initial__File__Creation(ForDname);
				strncpy(temp__file->Rsibling->content, copy__file->content, 1024);
			}
			else {
				search__file = temp__file->Rsibling;
				while (search__file->Rsibling != NULL) {
					search__file = search__file->Rsibling;
				}
				temp__file = Initial__File__Creation(ForDname);
				strncpy(temp__file->content, copy__file->content, 1024);
				search__file->Rsibling = temp__file;
			}
		}
		dtree->Cnode = current__dir;
	}
}

// Command 7) rm
void Explore__Directory(tree* dtree, char* findname, int option) {
	char YESORNO;
	directory* rmdir;
	if (option == 0) {
		if (Function__File(dtree, findname, option) == FALSE) {
			rmdir = Leave__Directory(dtree, findname);
			if (rmdir != NULL)
				printf("removing directory is not possible\n");
		}
	}

	else if (option == 1) {
		if (Function__File(dtree, findname, option) == FALSE) {
			rmdir = Leave__Directory(dtree, findname);
			if (rmdir != NULL)
				printf("removing directory is not possible\n");
		}
	}

	else if (option == 2) {
		if (Function__File(dtree, findname, option) == FALSE) {
			rmdir = Leave__Directory(dtree, findname);
			if (rmdir != NULL) {
				Function__rmdir(structure, findname, option);
			}
		}
	}
	else if (option == 3) {
		if (Function__File(dtree, findname, option) == FALSE) {
			rmdir = Leave__Directory(dtree, findname);
			if (rmdir != NULL) {
				Function__rmdir(structure, findname, option);
			}
		}
	}
}

int Function__File(tree* dtree, char* findname, int option) {
	file* rmfile = NULL;
	file* find__file = NULL;
	file* prev__file = NULL;

	find__file = dtree->Cnode->infile;
	prev__file = dtree->Cnode->infile;

	char YESORNO;

	int rm__case;

	if (prev__file == NULL)
		return FALSE;

	else if (strcmp(find__file->name, findname) == 0) {
		dtree->Cnode->infile = find__file->Rsibling;
		rmfile = find__file;
		rm__case = 0;
	}
	else {
		rm__case = 1;
		find__file = find__file->Rsibling;

		while (find__file != NULL) {
			if (strcmp(find__file->name, findname) == 0) {
				rmfile = find__file;
				break;
			}
			else {
				prev__file = find__file;
				find__file = find__file->Rsibling;
			}
		}
	}

	if (rmfile != NULL) {
		if ((option == 0) || (option == 2)) {
			printf("Would you wanna remove this file? : ");
			scanf("%c", &YESORNO);

			if (YESORNO == 'y') {
				prev__file->Rsibling = rmfile->Rsibling;
				free(rmfile);
				return TRUE;
			}

			else {
				if (rm__case == 0)
					dtree->Cnode->infile = rmfile;
				return FALSE;
			}
		}

		else {
			prev__file->Rsibling = rmfile->Rsibling;
			free(rmfile);
			return TRUE;
		}
	}
	else {
		if ((option == 2) && (option == 3)) {
			printf("That file don't exist in this directory\n");
			return FALSE;
		}
		return FALSE;
	}
}

// clear director's node (Recursive)
void rm__subdir(directory* dir) {
	if (dir != NULL) {
		rm__subdir(dir->Lchild);
		rm__subdir(dir->Rsibling);
		free(dir);
	}
}

//// Additional Commands
// 1) rmdir (delete directory)
int Function__rmdir(tree* dtree, char* finddirname, int option) {
	directory* rmdir = NULL;
	directory* find__dir = NULL;
	directory* prev__dir = NULL;

	find__dir = dtree->Cnode->Lchild;
	prev__dir = dtree->Cnode->Lchild;

	char YESORNO;

	int rm__case;

	if (prev__dir == NULL) {
		return FALSE;
	}

	else if (strcmp(find__dir->name, finddirname) == 0) {
		dtree->Cnode->Lchild = find__dir->Rsibling;
		rmdir = find__dir;
		rm__case = 0;
	}
	else {
		rm__case = 1;
		find__dir = find__dir->Rsibling;
		while (find__dir != NULL)
		{
			if (strcmp(find__dir->name, finddirname) == 0)
			{
				rmdir = find__dir;
				break;
			}
			else
			{
				prev__dir = find__dir;
				find__dir = find__dir->Rsibling;
			}
		}
	}

	if (rmdir != NULL) {
		if (option == 2) {
			printf("Would you wanna remove this Directory? : ");
			scanf("%c", &YESORNO);
			if (YESORNO == 'y') {
				if (rm__case == 1) {
					prev__dir->Rsibling = rmdir->Rsibling;
					rm__subdir(rmdir->Lchild);
					free(rmdir);
				}
				else {
					rm__subdir(rmdir->Lchild);
					free(rmdir);
				}
			}
			else {
				if (rm__case == 0) {
					dtree->Cnode->Lchild = rmdir;
				}
			}
		}
		else {
			if (rm__case == 1) {
				prev__dir->Rsibling = rmdir->Rsibling;
				rm__subdir(rmdir->Lchild);
				free(rmdir);
			}
			else {
				rm__subdir(rmdir->Lchild);
				free(rmdir);
			}
		}
	}
	else {
		printf("That directory don't exist!!\n");
	}
}

// 2) chmod
// change file or directory's access authority
void chmod__(tree* dtree, int permission, char* name) {
	directory* temp__node = Leave__Directory(structure, name);
	file* temp__file = Leave__File(structure, name);
	if (temp__node != NULL)
		temp__node->chmod__ = permission;
	else if (temp__file != NULL)
		temp__file->chmod__ = permission;
	else
		printf("error exists : Directory or File doesn't exit!\n");
}


// 3) Function__touch
// create a blank file and update file's date, time
void Function__touch(tree* dtree, char* filename, int option) {
	file* temp__file;
	file* current__file = dtree->Cnode->infile;
	directory* temp__node = dtree->Cnode;


	// case1) create a blank file
	if (option == 0) {
		temp__file = Leave__File(structure, filename);
		if (temp__file == NULL) {
			if (temp__node->infile == NULL) {
				temp__file = Initial__File__Creation(filename);
				temp__node->infile = temp__file;
			}
			else {
				if (temp__node->infile->Rsibling == NULL) {
					temp__file = Initial__File__Creation(filename);
					temp__node->infile->Rsibling = temp__file;
				}
				else {
					current__file = current__file->Rsibling;

					while (current__file->Rsibling != NULL)
						current__file = current__file->Rsibling;
					temp__file = Initial__File__Creation(filename);
					current__file->Rsibling = temp__file;
				}
			}
		}
		else {
			time_t timer;
			timer = time(NULL);
			temp__file->t = localtime(&timer);
		}
	}
	// set file's date and time
	else {
		temp__file = Leave__File(structure, filename);
		if (temp__file != NULL) {
			time_t timer;
			timer = time(NULL);
			temp__file->t = localtime(&timer);
		}
	}
}

// update file's time
void touch__timer(tree* dtree, char* filetime, char* filename) {
	char str__time[2];
	file* temp__file;
	temp__file = Leave__File(structure, filename);
	if (temp__file != NULL) {
		int minute;
		int second;
		int month;
		int day;
		int hour;

		strncpy(str__time, filetime + 4, 2);
		month = atoi(str__time);
		if (month > 12) {
			printf("Time is not correct!!\n");
			return;
		}
		strncpy(str__time, filetime + 6, 2);
		day = atoi(str__time);
		if (day > 31) {
			printf("Time is not correct!!\n");
			return;
		}
		strncpy(str__time, filetime + 8, 2);
		hour = atoi(str__time);
		if (hour > 23) {
			printf("Time is not correct!!\n");
			return;
		}
		strncpy(str__time, filetime + 10, 2);
		minute = atoi(str__time);
		if (minute > 60) {
			printf("Time is not correct!!\n");
			return;
		}
		strncpy(str__time, filetime + 12, 2);

		second = atoi(str__time);
		if (second > 60) {
			printf("Time is not correct!!\n");
			return;
		}
		temp__file->t->tm_mon = month;
		temp__file->t->tm_mday = day;
		temp__file->t->tm_hour = hour;
		temp__file->t->tm_min = minute;
		temp__file->t->tm_sec = second;
	}
	return 0;
}


// Assemble of every command's for user
// Option 0 : Basic
// Option 1~ : Specific

// Command
void User__Command(char* str, int cmdoption) {
	char temp[50];
	char* command = NULL;

	directory* temp__dir = NULL;
	file* temp__file = NULL;

	char* Dir__Path;
	char* temp__str;


	int i;
	int h = 0;
	int chtemp;

	int option;
	int Save__Command = 0;
	strncpy(temp, str, 30);

	// Enter the File Explorer
	if (strcmp(str, "") == 0)
		return;
	command = strtok(str, " ");

	// Command 1) mkdir
	if (strcmp(command, "mkdir") == 0) {
		Save__Command = 1;
		char* tempstr;
		command = strtok(NULL, " ");

		if (command == NULL)
			printf("Nothing was entered in User's Command\n");
		else {
			// when creating a Directory, User authorithy is allocated
			if (strcmp(command, "-m") == 0) {
				option = 1;
				command = strtok(NULL, " ");
				chtemp = atoi(command);
				command = strtok(NULL, " ");
				temp__dir = Function__mkdir(structure, command, option);
				temp__dir->chmod__ = chtemp;
			}

			// when creating a Directory, Upper directory is created with lower directory
			// ex) mkdir -p abc/def/ghi
			else if (strcmp(command, "-p") == 0) {
				option = 2;
				command = strtok(NULL, " ");
				tempstr = command;
				// present current node saving situation
				temp__dir = structure->Cnode;

				Dir__Path = strtok(command, "/");

				while (Dir__Path != NULL) {
					if (Function__cd(structure, Dir__Path) == FALSE) {
						Function__mkdir(structure, Dir__Path, 2);
						Function__cd(structure, Dir__Path);
					}
					Dir__Path = strtok(NULL, "/");
				}
				structure->Cnode = temp__dir;
			}
			else {
				option = 0;
				// when creating Directory x, y, z, present x/y/z
				if (strchr(command, '/') != NULL) {
					temp__dir = structure->Cnode;
					Dir__Path = strrchr(command, '/');
					Dir__Path = Dir__Path + 1;
					command = strtok(command, "/");
					while (command != Dir__Path) {
						if (Function__cd(structure, command) == FALSE) {
							printf("Can't exit this Directory\n");
							break;
						}
						command = strtok(NULL, "/");
					}
					if (Dir__Path == command)
						Function__mkdir(structure, command, 0);
					structure->Cnode = temp__dir;
				}
				// Multi Processing
				else {
					int state;
					int fd[2];
					int k = -1;
					char size_buffer[200];
					pid_t process__id[3];
					state = pipe(fd);

					state = sem_init(&semp, 1, 1);
					while (command != NULL) {
						if (k < 3) {
							process__id[++k] = fork(); // make child process
						}
						write(fd[1], size_buffer, 200);

						if (process__id[k] == 0) {
							sem_wait(&semp);
							read(fd[0], size_buffer, 200);
							Function__mkdir(structure, command, option);
							command = strtok(NULL, " ");
							write(fd[1], size_buffer, 200);
							sem_post(&semp);
						}
						else if (process__id[k] > 0)
							wait(&state);
						else
							printf("Error Exist!\n");
					}
					sem_destroy(&semp);
				}
			}
		}
	}

	// Command 2) pwd
	else if (strcmp(command, "pwd") == 0) {
		Function__pwd(structure, qstack, 0);
		Save__Command = 0;
	}

	// Command 3) cd
	else if (strcmp(command, "cd") == 0) {
		Save__Command = 1;
		command = strtok(NULL, " ");
		Change__Directory__position(structure, command);
	}
	else if ((strcmp(command, "cat") == 0) && (cmdoption == 1)) {
		command = strtok(NULL, " ");
		if (strcmp(command, ">") == 0) {
			Save__Command = 1;
			option = 1;
			command = strtok(NULL, " ");
			Function__cat(structure, command, option);
			temp__file = Leave__File(structure, command);
		}
		else {
			option = 0;
			Function__cat(structure, command, option);
		}
	}

	// Command 4) cat
	else if ((strcmp(command, "cat") == 0) && (cmdoption == 0)) {
		char* cat_content;
		char* f_name;
		int len;
		command = strtok(NULL, " ");
		command = strtok(NULL, " ");
		cat_content = command;
		command = strtok(NULL, " ");
		len = command - cat_content;
		char* cat_filename = (char*)malloc(len);
		strncpy(cat_filename, cat_content, len);
		*(cat_filename + len - 1) = '\0';
		Initial__cat(structure, cat_filename, command);
	}

	// Command 5) ls
	else if (strcmp(command, "ls") == 0) {
		if (strchr(temp, '-') == NULL) {
			option = 0;
			Function__ls(structure, option);
		}
		else {
			command = strtok(NULL, " ");
			// ls : "ls-l"
			if (strcmp(command, "-l") == 0) {
				option = 1;
				Function__ls(structure, option);
			}
			// ls : "ls-a"
			else if (strcmp(command, "-a") == 0) {
				option = 2;
				Function__ls(structure, option);
			}
			// ls : "ls-al"
			else {
				option = 3;
				Function__ls(structure, option);
			}
		}
	}

	// Command 6) cp
	else if (strcmp(command, "cp") == 0) {
		command = strtok(NULL, " ");
		temp__str = command;
		command = strtok(NULL, " ");
		Function__cp(structure, temp__str, command);
	}

	// Command 7) rm
	else if (strcmp(command, "rm") == 0) {
		Save__Command = 1;
		command = strtok(NULL, " ");

		// rm : "rm-f"
		if (strcmp(command, "-f") == 0) {
			option = 1;
			command = strtok(NULL, " ");
			Explore__Directory(structure, command, option);
		}

		// rm : "rm-r"
		else if (strcmp(command, "-r") == 0) {
			option = 2;
			command = strtok(NULL, " ");
			Explore__Directory(structure, command, option);
		}

		// rm : "rm-rf"
		else if (strcmp(command, "-rf") == 0) {
			option = 3;
			command = strtok(NULL, " ");
			Explore__Directory(structure, command, option);
		}

		// rm : "rm"
		else {
			option = 0;
			Explore__Directory(structure, command, option);
		}

	}



	// Additional Command 1) rmdir
	else if (strcmp(command, "rmdir") == 0) {
		Save__Command = 1;
		option = 2;
		command = strtok(NULL, " ");
		Explore__Directory(structure, command, option);
	}

	// Additional Command 2) chmod
	else if (strcmp(command, "chmod") == 0) {
		Save__Command = 1;
		command = strtok(NULL, " ");
		chtemp = atoi(command);
		command = strtok(NULL, " ");
		chmod__(structure, chtemp, command);
	}

	// Additional Command 3) touch
	else if (strcmp(command, "touch") == 0) {
		command = strtok(NULL, " ");
		if (strcmp(command, "-t") == 0) {
			command = strtok(NULL, " ");
			temp__str = command;
			command = strtok(NULL, " ");
			touch__timer(structure, temp__str, command);
		}
		else if (strcmp(command, "-c") == 0) {
			option = 2;
			command = strtok(NULL, " ");
			Function__touch(structure, command, option);
		}
		else {
			option = 0;
			Function__touch(structure, command, option);
		}
	}

	// When wrong command's were entered
	else
		printf("Wrong command is entered\n");

	// update's file "saving.txt"
	if ((Save__Command == 1) && (cmdoption == 1)) {
		FILE* fp = fopen("saving.txt", "a");
		if (strstr(temp, "cat >") == NULL) {
			fputs(temp, fp);
			fputs("\n", fp);
		}
		else {
			fputs(temp, fp);
			fputs(" ", fp);
			fputs(temp__file->content, fp);
		}
		fclose(fp);
	}
}
