/*
 * pets.c - pet allocation, per-type data, and list cleanup.
 */
#include <linux/slab.h>
#include "elevator.h"

/* Indexed by type: 0 = Chihuahua, 1 = Pug, 2 = Pughuahua, 3 = Dachshund */
static const int pet_weights[NUM_PET_TYPES] = { 3, 14, 10, 16 };
static const char pet_letters[NUM_PET_TYPES] = { 'C', 'P', 'H', 'D' };

/* Allocate a pet with kmalloc. Caller validates the arguments. */
struct pet *pet_create(int destination, int type, unsigned long long order)
{
    struct pet *new_pet = kmalloc(sizeof(*new_pet), GFP_KERNEL);

    if (!new_pet)
        return NULL;

    new_pet->destination = destination;
    new_pet->type = type;
    new_pet->weight = pet_weights[type];
    new_pet->order = order;
    INIT_LIST_HEAD(&new_pet->list);
    return new_pet;
}

char pet_letter(int type)
{
    return pet_letters[type];
}

/* Remove and free every pet in a list. Caller holds elevator_lock. */
void pet_free_list(struct list_head *head)
{
    struct pet *pet, *next;

    list_for_each_entry_safe(pet, next, head, list) {
        list_del(&pet->list);
        kfree(pet);
    }
}
