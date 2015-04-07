#include "stuck.h"

Item* Stuck::search_item (string item_name) {
	cerr << all_items.size();
	for (int i = 0;i<all_items.size();i++) {
		if (all_items[i] -> get_item_name() == item_name)
			return all_items[i];
	}
	return new Item ("NO Such Item!" ,0);
}

void Stuck::print_list () {
	for (int i = 0; i < all_items.size(); ++i)
	{
		cout << all_items[i] -> get_item_name() << " " << all_items[i] -> get_item_amount() << endl;
	}
}