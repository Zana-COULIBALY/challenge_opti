#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ITEMS 10000
#define MAX_TRUCK_TYPES 1000
#define MAX_TRUCKS 10000
#define MAX_PILES 10000
#define MAX_ITEMS_PER_PILE 1000
#define MAX_SUPPLIERS_PER_TRUCK 100

typedef struct {
    int id;
    int supplier;
    int length, width, height, weight;
    int stackability_code;
    int earliest_arrival, latest_arrival;
    int inventory_cost;
} Item;

typedef struct {
    int id;
    int arrival_time;
    int length, width, height, max_weight, cost;
} TruckType;

typedef struct {
    int x, y; // position in the truck
    int length, width; // dimensions of the pile
    int height; // total height of the pile
    int weight; // total weight of the pile
    int stackability_code; // stackability code for the pile
    int item_ids[MAX_ITEMS_PER_PILE];
    int num_items;
} Pile;

typedef struct {
    int truck_type_id;
    int arrival_time;
    int current_weight;
    int num_piles;
    Pile piles[MAX_PILES];
} TruckInstance;

typedef struct {
    int truck_type_id;
    int num_suppliers;
    int suppliers[MAX_SUPPLIERS_PER_TRUCK];
} SupplierTruck;

Item items[MAX_ITEMS];
TruckType truck_types[MAX_TRUCK_TYPES];
SupplierTruck supplier_trucks[MAX_TRUCK_TYPES];
TruckInstance trucks[MAX_TRUCKS];
int num_items = 0;
int num_truck_types = 0;
int num_supplier_trucks = 0;
int num_trucks = 0;

// Function to read items from file
void read_items(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    char buffer[256];
    fgets(buffer, sizeof(buffer), file); // Skip header line
    fscanf(file, "%*s %d", &num_items); // Read the number of items
    for (int i = 0; i < num_items; i++) {
        fscanf(file, "%d %d %d %d %d %d %d %d %d",
               &items[i].id,
               &items[i].supplier,
               &items[i].length,
               &items[i].width,
               &items[i].height,
               &items[i].weight,
               &items[i].stackability_code,
               &items[i].earliest_arrival,
               &items[i].latest_arrival);
        fscanf(file, "%d", &items[i].inventory_cost);
    }
    fclose(file);
}

// Function to read truck types from file
void read_truck_types(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    char buffer[256];
    fgets(buffer, sizeof(buffer), file); // Skip header line
    fscanf(file, "%*s %d", &num_truck_types); // Read the number of truck types
    for (int i = 0; i < num_truck_types; i++) {
        fscanf(file, "%d %d %d %d %d %d %d",
               &truck_types[i].id,
               &truck_types[i].arrival_time,
               &truck_types[i].length,
               &truck_types[i].width,
               &truck_types[i].height,
               &truck_types[i].max_weight,
               &truck_types[i].cost);
    }
    fclose(file);
}

// Function to read supplier-truck relationships from file
void read_supplier_trucks(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    char buffer[256];
    fgets(buffer, sizeof(buffer), file); // Skip header line
    fscanf(file, "%*s %d", &num_supplier_trucks); // Read the number of entries
    for (int i = 0; i < num_supplier_trucks; i++) {
        int truck_type_id, num_suppliers;
        fscanf(file, "%d %d", &truck_type_id, &num_suppliers);
        supplier_trucks[i].truck_type_id = truck_type_id;
        supplier_trucks[i].num_suppliers = num_suppliers;
        for (int j = 0; j < num_suppliers; j++) {
            fscanf(file, "%d", &supplier_trucks[i].suppliers[j]);
        }
    }
    fclose(file);
}

// Function to check if a truck type can pick up from a supplier
int is_supplier_compatible(int truck_type_id, int supplier) {
    for (int i = 0; i < num_supplier_trucks; i++) {
        if (supplier_trucks[i].truck_type_id == truck_type_id) {
            for (int j = 0; j < supplier_trucks[i].num_suppliers; j++) {
                if (supplier_trucks[i].suppliers[j] == supplier) {
                    return 1;
                }
            }
            return 0;
        }
    }
    return 0; // If truck_type_id not found, assume incompatible (shouldn't happen)
}

// Function to check if an item can be added to a pile
int can_add_to_pile(Pile *pile, Item item, TruckType truck_type) {
    if (item.stackability_code != pile->stackability_code) return 0;
    if (item.length != pile->length || item.width != pile->width) return 0;
    if (pile->weight + item.weight > truck_type.max_weight) return 0;
    if (pile->height + item.height > truck_type.height) return 0;
    return 1;
}

// Function to find space for a new pile in a truck
int find_space_for_pile(TruckInstance *truck, int length, int width, int *x, int *y) {
    TruckType truck_type = truck_types[truck->truck_type_id];
    for (int y_candidate = 0; y_candidate <= truck_type.width - width; y_candidate++) {
        for (int x_candidate = 0; x_candidate <= truck_type.length - length; x_candidate++) {
            int overlap = 0;
            for (int p = 0; p < truck->num_piles; p++) {
                Pile pile = truck->piles[p];
                if (!(x_candidate + length <= pile.x ||
                      x_candidate >= pile.x + pile.length ||
                      y_candidate + width <= pile.y ||
                      y_candidate >= pile.y + pile.width)) {
                    overlap = 1;
                    break;
                }
            }
            if (!overlap) {
                *x = x_candidate;
                *y = y_candidate;
                return 1;
            }
        }
    }
    return 0;
}

// Function to add an item to an existing pile
void add_to_pile(Pile *pile, Item item) {
    pile->item_ids[pile->num_items] = item.id;
    pile->num_items++;
    pile->height += item.height;
    pile->weight += item.weight;
}

// Function to create a new pile in a truck
void create_new_pile(TruckInstance *truck, Item item, int x, int y) {
    Pile new_pile;
    new_pile.x = x;
    new_pile.y = y;
    new_pile.length = item.length;
    new_pile.width = item.width;
    new_pile.height = item.height;
    new_pile.weight = item.weight;
    new_pile.stackability_code = item.stackability_code;
    new_pile.item_ids[0] = item.id;
    new_pile.num_items = 1;
    truck->piles[truck->num_piles] = new_pile;
    truck->num_piles++;
    truck->current_weight += item.weight;
}

// Function to find a compatible truck type for an item
int find_compatible_truck_type(Item item) {
    for (int i = 0; i < num_truck_types; i++) {
        if (!is_supplier_compatible(truck_types[i].id, item.supplier)) continue;
        if (truck_types[i].max_weight >= item.weight &&
            truck_types[i].length >= item.length &&
            truck_types[i].width >= item.width &&
            truck_types[i].height >= item.height) {
            return i;
        }
    }
    return -1; // No compatible truck type found
}

// Function to create a new truck instance
void create_new_truck(Item item) {
    int truck_type_id = find_compatible_truck_type(item);
    if (truck_type_id == -1) {
        fprintf(stderr, "No compatible truck type found for item %d\n", item.id);
        exit(EXIT_FAILURE);
    }
    TruckInstance new_truck;
    new_truck.truck_type_id = truck_type_id;
    new_truck.arrival_time = truck_types[truck_type_id].arrival_time;
    new_truck.current_weight = 0;
    new_truck.num_piles = 0;
    trucks[num_trucks] = new_truck;

    // Find space for the new pile (which will contain only this item for now)
    int x, y;
    if (find_space_for_pile(&trucks[num_trucks], item.length, item.width, &x, &y)) {
        create_new_pile(&trucks[num_trucks], item, x, y);
    } else {
        fprintf(stderr, "No space found for item %d in new truck\n", item.id);
        exit(EXIT_FAILURE);
    }
    num_trucks++;
}

// Main function to assign items to trucks
void assign_items_to_trucks() {
    for (int i = 0; i < num_items; i++) {
        Item item = items[i];
        int placed = 0;

        // Try to place in existing trucks
        for (int j = 0; j < num_trucks; j++) {
            TruckInstance *truck = &trucks[j];
            TruckType truck_type = truck_types[truck->truck_type_id];

            // Check if the truck can pick up from the item's supplier
            if (!is_supplier_compatible(truck_type.id, item.supplier)) continue;

            // Check arrival time constraint
            if (item.earliest_arrival > truck_type.arrival_time ||
                item.latest_arrival < truck_type.arrival_time) {
                continue;
            }

            // Try to add to existing piles
            for (int p = 0; p < truck->num_piles; p++) {
                Pile *pile = &truck->piles[p];
                if (can_add_to_pile(pile, item, truck_type)) {
                    add_to_pile(pile, item);
                    placed = 1;
                    break;
                }
            }
            if (placed) break;

            // Try to create a new pile in this truck
            int x, y;
            if (find_space_for_pile(truck, item.length, item.width, &x, &y) &&
                truck->current_weight + item.weight <= truck_type.max_weight) {
                create_new_pile(truck, item, x, y);
                placed = 1;
                break;
            }
        }

        // If not placed yet, create a new truck
        if (!placed) {
            create_new_truck(item);
        }
    }
}

// Function to calculate total cost
int calculate_total_cost() {
    int total_cost = 0;

    // Add truck costs
    for (int j = 0; j < num_trucks; j++) {
        int truck_type_id = trucks[j].truck_type_id;
        total_cost += truck_types[truck_type_id].cost;
    }

    // Add storage costs for each item
    for (int i = 0; i < num_items; i++) {
        int truck_arrival_time = -1;
        for (int j = 0; j < num_trucks; j++) {
            for (int p = 0; p < trucks[j].num_piles; p++) {
                for (int k = 0; k < trucks[j].piles[p].num_items; k++) {
                    if (trucks[j].piles[p].item_ids[k] == items[i].id) {
                        truck_arrival_time = truck_types[trucks[j].truck_type_id].arrival_time;
                        break;
                    }
                }
                if (truck_arrival_time != -1) break;
            }
            if (truck_arrival_time != -1) break;
        }
        if (truck_arrival_time != -1) {
            int arrival_diff = items[i].latest_arrival - truck_arrival_time;
            if (arrival_diff > 0) {
                total_cost += arrival_diff * items[i].inventory_cost;
            }
        }
    }

    return total_cost;
}

// Function to write the solution to a file
void write_solution(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Write header
    fprintf(file, "EQUIPE MyTeam\n");
    fprintf(file, "INSTANCE 1\n"); // Assuming instance 1 for now

    // Write truck and pile information
    for (int j = 0; j < num_trucks; j++) {
        fprintf(file, "TYPECAMION %d\n", trucks[j].truck_type_id);
        for (int p = 0; p < trucks[j].num_piles; p++) {
            Pile pile = trucks[j].piles[p];
            fprintf(file, "PILE %d %d %d %d\n", pile.length, pile.width, pile.x, pile.y);
            fprintf(file, "OBJETS");
            for (int k = 0; k < pile.num_items; k++) {
                fprintf(file, " %d", pile.item_ids[k]);
            }
            fprintf(file, "\n");
        }
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <items_file> <trucks_file> <supplier_trucks_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Read input files
    read_items(argv[1]);
    read_truck_types(argv[2]);
    read_supplier_trucks(argv[3]);

    // Assign items to trucks
    assign_items_to_trucks();

    // Calculate and print total cost
    int total_cost = calculate_total_cost();
    printf("Total cost: %d\n", total_cost);

    // Write solution to file
    write_solution("res_1.txt");

    return EXIT_SUCCESS;
}
