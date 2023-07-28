#define CAPACITY 50000 // Size of the Hash Table

#define HIGH_DDR_ADDR  0x800000000UL
#define HIGH_DDR_ADDR2 0x840000000UL

// Hashtable implementation starts
unsigned long hash_function(T key) {
    return key % CAPACITY;
}
 
typedef struct Ht_item Ht_item;
 
struct Ht_item {
    T key;
    T *value;
};
 
 
typedef struct LinkedList LinkedList;

struct LinkedList {
    Ht_item* item; 
    LinkedList* next;
};
 
 
typedef struct HashTable HashTable;
 
struct HashTable {
    Ht_item** items;
    LinkedList** overflow_buckets;
    int size;
    int count;
};

static LinkedList* allocate_list () {
    LinkedList* list = (LinkedList*) malloc (sizeof(LinkedList));
    return list;
}
 
static LinkedList* linkedlist_insert(LinkedList* list, Ht_item* item) {
    if (!list) {
        LinkedList* head = allocate_list();
        head->item = item;
        head->next = NULL;
        list = head;
        return list;
    } 
     
    else if (list->next == NULL) {
        LinkedList* node = allocate_list();
        node->item = item;
        node->next = NULL;
        list->next = node;
        return list;
    }
 
    LinkedList* temp = list;
    while (temp->next->next) {
        temp = temp->next;
    }
     
    LinkedList* node = allocate_list();
    node->item = item;
    node->next = NULL;
    temp->next = node;
     
    return list;
}

static void free_linkedlist(LinkedList* list) {
    LinkedList* temp = list;
    while (list) {
        temp = list;
        list = list->next;
        free(temp->item->value);
        free(temp->item);
        free(temp);
    }
}

static void free_overflow_buckets(HashTable* table) {
    LinkedList** buckets = table->overflow_buckets;
    for (int i=0; i<table->size; i++)
        free_linkedlist(buckets[i]);
    free(buckets);
}

 
static LinkedList** create_overflow_buckets(HashTable* table) {
    LinkedList** buckets = (LinkedList**) calloc (table->size, sizeof(LinkedList*));
    for (int i=0; i<table->size; i++)
        buckets[i] = NULL;
    return buckets;
}

Ht_item* create_item(T key, T *value, int count) {
    Ht_item* item = (Ht_item*) malloc (sizeof(Ht_item));
    item->key = key;
    item->value = (T *)malloc(sizeof(T)*count);
     
    for (int i = 0; i < count; i++)
    {
        *(item->value + i) = *(value + i);
    }
    return item;
}
 
HashTable* create_table(int size) {
    HashTable* table = (HashTable*) malloc (sizeof(HashTable));
    table->size = size;
    table->count = 0;
    table->items = (Ht_item**) calloc (table->size, sizeof(Ht_item*));
    for (int i=0; i<table->size; i++)
        table->items[i] = NULL;
    table->overflow_buckets = create_overflow_buckets(table);
    return table;
}

void free_item(Ht_item* item) {
    free(item->value);
    free(item);
}
 
void free_table(HashTable* table) {
    for (int i=0; i<table->size; i++) {
        Ht_item* item = table->items[i];
        if (item != NULL)
            free_item(item);
    }
 
    free_overflow_buckets(table);
    free(table->items);
    free(table);
}

void handle_collision(HashTable* table, unsigned long index, Ht_item* item) {
    LinkedList* head = table->overflow_buckets[index];
    if (head == NULL) {
        head = allocate_list();
        head->item = item;
        table->overflow_buckets[index] = head;
        return;
    }
    else {
        table->overflow_buckets[index] = linkedlist_insert(head, item);
        return;
    }
 }

 void ht_insert(HashTable* table, T key, T *value, int count) {
    Ht_item* item = create_item(key, value, count);
    unsigned long index = hash_function(key);
    Ht_item* current_item = table->items[index];
     
    if (current_item == NULL) {
        if (table->count == table->size) {
            printf("\nInsert Error: Hash Table is full!!!\n");
            exit(1);
            return;
        }
        table->items[index] = item; 
        table->count++;
    }
 
    else {
        if (current_item->key == key) {
            // printf("%d", key);
            // printf("\nKey already exist!!!\n");
            // exit(1);
            for (int i = 0; i < count; i++)
            {
                *(item->value + i) = *(value + i);
            }
            return;
        }
        else {
            handle_collision(table, index, item);
            return;
        }
    }
}

T* ht_search(HashTable* table, T key) {
    int index = hash_function(key);
    Ht_item* item = table->items[index];
    LinkedList* head = table->overflow_buckets[index];

    while (item != NULL) {
        if (item->key == key)
            return item->value;
        if (head == NULL)
            return NULL;
        item = head->item;
        head = head->next;
    }
    return NULL;
}
 
void print_search(HashTable* table, T key) {
    T* val;
    if ((val = ht_search(table, key)) == NULL) {
        printf("%d does not exist\n", key);
        return;
    }
    else {
        printf("Key:%d\n", key);
        printf("Value: ");
        for (int i = 0; i < 2; i++)
        {
            printf("%d ", *(val + i));
        }
        printf("\n");
    }
}

void print_table(HashTable* table, int enabled_col_num) {
    printf("\n-------------------\n");
    for (int i=0; i < table->size; i++) {
        if (table->items[i]) {
            printf("Index:%d, Key:%d, Value: ", i, table->items[i]->key);
            for (int j = 0; j < (enabled_col_num - 1); j++)
            {
              printf("%d ", *(table->items[i]->value + j));
            }
            
            if (table->overflow_buckets[i]) {
                printf(" => Overflow Bucket => ");
                LinkedList* head = table->overflow_buckets[i];
                while (head) {
                    printf("Key:%d", head->item->key);
                    head = head->next;
                }
            }
            printf("\n");
        }
    }
    printf("-------------------\n");
}
// Hashtable implementation finishe

#define __dsb(){\
  do{\
    asm volatile("dsb 15");\
  }while(0);\
}

