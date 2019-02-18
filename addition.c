#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/sort.h>
#include <linux/string.h>
#include <linux/slab.h>


//static char *animals[] = {"cat", "dog", "horse", "dog", "lion", "tiger", "frog", "cat", "deer", "boar", "hippo", "snake", "wolf", "fox", "koala", "panda", "elephant", "boar", "hippo", "fox", "hippo", "frog", "dodo", "bison", "crab", "rabbit", "emu", "goat", "kiwi", "lion", "pig", "pike", "mole", "mule", "rat", "swan", "falcon", "ant", "mouse", "hare", "swan", "duck", "frog", "tiger", "panda", "dodo", "crab", "cat", "tiger", "frog"};

static char *animals[10];
static int size_of_each_element = sizeof(animals[0]);
static int no_of_elements = sizeof(animals)/sizeof(animals[0]);

struct list_of_animals{
	struct list_head lion;
	char * animal_type;
	int no_of_resp_type;
}animal_list, animal_filtered_list;

char* animal_type_filter = "none";
int animal_count_filter = 0;
int zero = 0;

module_param(animal_count_filter, int, 0644);
MODULE_PARM_DESC(animal_count_filter,"Filter the animals whose count exceeds the filter value");
module_param(animal_type_filter, charp, 0644);
MODULE_PARM_DESC(animal_type_filter,"Filter the animals whose name is same as entered");
module_param_array(animals,charp,&zero,0644);

//MODULE_PARM_DESC(animals,"Enter the animals in the database");


static int compare(const void*, const void*);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sanika");
MODULE_DESCRIPTION("Module for sorting using list");


struct list_of_animals *temp_animal_store, *temp_animal;
struct list_head *access;
char* previous_animal[1];
int an_ct = 1, no_of_nodes = 0, number_of_filtered = 0;

static int list_init(void)
{
	int i;
	printk("size_of_each_element = %d",size_of_each_element);
	printk("no_of_elements = %d", no_of_elements);
	sort(animals,no_of_elements,size_of_each_element,compare,NULL);
	INIT_LIST_HEAD(&animal_list.lion);
	INIT_LIST_HEAD(&animal_filtered_list.lion);
	
	for(i=0; i<no_of_elements; i++)
	{
		if(i < (no_of_elements -1))
		{
			if(strcmp(animals[i],animals[i+1]) == 0)
			{
				an_ct ++;
				previous_animal[0] = animals[i];
			}
			else
			{
				temp_animal_store = (struct list_of_animals *)kmalloc(sizeof(animal_list),GFP_KERNEL);
				temp_animal_store->animal_type = animals[i];
				temp_animal_store->no_of_resp_type = an_ct;
				//printk("animal = %s and number = %d",temp_animal_store->animal_type, temp_animal_store->no_of_resp_type);
				list_add_tail(&(temp_animal_store->lion),&(animal_list.lion));
				
				an_ct = 1;
				no_of_nodes ++;
			}
		}
	}
	temp_animal_store = (struct list_of_animals *)kmalloc(sizeof(animal_list),GFP_KERNEL);
	temp_animal_store->animal_type = animals[no_of_elements-1];
	temp_animal_store->no_of_resp_type = an_ct;
	printk("animal = %s and number = %d",temp_animal_store->animal_type, temp_animal_store->no_of_resp_type);
	list_add_tail(&(temp_animal_store->lion),&(animal_list.lion));
	no_of_nodes ++;

	printk("The ecosystem is ready");
	list_for_each(access, &animal_list.lion)
	{
		temp_animal_store = list_entry(access, struct list_of_animals, lion);
		printk("animal_type = %s and count = %d",temp_animal_store->animal_type,temp_animal_store->no_of_resp_type);
		//previous_animal[0] = *animal_type_filter;
		//if(strcmp(*animal_type_filter,"standard") == 0)
		//printk("%d",strcmp(temp_animal_store->animal_type,animal_type_filter));
		if(strcmp(animal_type_filter,"none")== 0)
		{
			if(temp_animal_store->no_of_resp_type > animal_count_filter)
			{
				temp_animal = (struct list_of_animals *)kmalloc(sizeof(animal_list),GFP_KERNEL);
				temp_animal->animal_type = temp_animal_store->animal_type;
				temp_animal->no_of_resp_type = temp_animal_store->no_of_resp_type;
				list_add_tail(&(temp_animal->lion),&(animal_filtered_list.lion));
				number_of_filtered++;
			}
		}
		else if((strcmp(temp_animal_store->animal_type,animal_type_filter)== 0) && (temp_animal_store->no_of_resp_type > animal_count_filter))
		{
			temp_animal = (struct list_of_animals *)kmalloc(sizeof(animal_list),GFP_KERNEL);
			temp_animal->animal_type = temp_animal_store->animal_type;
			temp_animal->no_of_resp_type = temp_animal_store->no_of_resp_type;
			list_add_tail(&(temp_animal->lion),&(animal_filtered_list.lion));
			number_of_filtered++;
		}
	}
	printk("Number of nodes is %d",no_of_nodes);
	printk("Amount of memory dynamically allocated = %d bytes",no_of_nodes*sizeof(animal_list));
	printk("After filtering the data");
	printk("Filter criteria is animal type = %s and count = %d",animal_type_filter,animal_count_filter);

	list_for_each(access, &animal_filtered_list.lion)
	{
		temp_animal = list_entry(access, struct list_of_animals, lion);
		printk("animal_type = %s and count = %d",temp_animal->animal_type,temp_animal->no_of_resp_type);
	}
	printk("Number of filtered animals is %d",number_of_filtered);
	printk("Amount of memory dynamically allocated = %d bytes",number_of_filtered*sizeof(animal_list));
	printk("\n");
	return 0;
}

static void list_exit(void)
{
	list_for_each(access, &animal_list.lion)
	{
		kfree(list_entry(access, struct list_of_animals, lion));
	}
	printk("Memory is free for first list");
	printk("Amount of memory freed = %d bytes",no_of_nodes*sizeof(animal_list));
	list_for_each(access, &animal_filtered_list.lion)
	{
		kfree(list_entry(access, struct list_of_animals, lion));
	}
	printk("Memory is free for second list");
	printk("Amount of memory freed = %d bytes",number_of_filtered*sizeof(animal_list));
	printk("Module removed successfully");
	printk("\n");
}

static int compare(const void* a, const void* b)
{
	return strcmp(*(const char**)a, *(const char**)b);
}

module_init(list_init);
module_exit(list_exit);
