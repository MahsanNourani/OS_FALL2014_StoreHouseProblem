#include "item.h"

void Item::add_item_to_stuck (int amount) {
	item_amount += amount;
}
void Item::remove_item_from_stuck (int amount) {
	item_amount -= amount;
}