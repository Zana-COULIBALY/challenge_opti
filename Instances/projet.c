#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct Pile {
    int x, y;
    int length, width;
    int height;
    int weight;
    int stackability_code;
    int *item_ids;
    int num_items;
    int capacity; // Current capacity of item_ids array
} Pile;

typedef struct TruckInstance {
    int truck_type_id;
    int arrival_time;
    int current_weight;
    int num_piles;
    Pile *piles;
    int piles_capacity; // Current capacity of piles array
} TruckInstance;

typedef struct {
    int truck_type_id;
    int num_suppliers;
    int *suppliers;
} SupplierTruck;

Item *items;
int num_items = 0;
TruckType *truck_types;
int num_truck_types = 0;
SupplierTruck *supplier_trucks;
int num_supplier_trucks = 0;
TruckInstance *trucks;
int num_trucks = 0;
int trucks_capacity = 0;

// Function to initialize or resize the items array
void init_items(int size) {
    items = (Item *)malloc(size * sizeof(Item));
    if (!items) {
        perror("Memory allocation failed for items");
        exit(EXIT_FAILURE);
    }
}

// Function to initialize or resize the truck_types array
void init_truck_types(int size) {
    truck_types = (TruckType *)malloc(size * sizeof(TruckType));
    if (!truck_types) {
        perror("Memory allocation failed for truck_types");
        exit(EXIT_FAILURE);
    }
}

// Function to initialize or resize the supplier_trucks array
void init_supplier_trucks(int size) {
    supplier_trucks = (SupplierTruck *)malloc(size * sizeof(SupplierTruck));
    if (!supplier_trucks) {
        perror("Memory allocation failed for supplier_trucks");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < size; i++) {
        supplier_trucks[i].suppliers = NULL;
    }
}

// Function to initialize or resize the trucks array
void init_trucks(int size) {
    trucks = (TruckInstance *)malloc(size * sizeof(TruckInstance));
    if (!trucks) {
        perror("Memory allocation failed for trucks");
        exit(EXIT_FAILURE);
    }
    trucks_capacity = size;
    for (int i = 0; i < size; i++) {
        trucks[i].piles = NULL;
        trucks[i].piles_capacity = 0;
        trucks[i].num_piles = 0;
    }
}

// Function to read items from file
void read_items(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    fscanf(file, "%*s %d", &num_items); // Read the number of items
    char buffer[256];
    fgets(buffer, sizeof(buffer), file);
    fgets(buffer, sizeof(buffer), file); // Skip header line
    init_items(num_items);
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

    // Debug print: list all items
    // printf("Read %d items:\n", num_items);
    // for (int i = 0; i < num_items; i++) {
    //     printf("Item %d: supplier=%d, length=%d, width=%d, height=%d, weight=%d, stack=%d, early=%d, late=%d, cost=%d\n",
    //            items[i].id, items[i].supplier, items[i].length, items[i].width,
    //            items[i].height, items[i].weight, items[i].stackability_code,
    //            items[i].earliest_arrival, items[i].latest_arrival, items[i].inventory_cost);
    // }
}

// Function to read truck types from file
void read_truck_types(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "%*s %d", &num_truck_types); // Read the number of truck types
    char buffer[256];
    fgets(buffer, sizeof(buffer), file);
    fgets(buffer, sizeof(buffer), file); // Skip header line
    init_truck_types(num_truck_types);
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

    // Debug print: list all truck types
    // printf("Read %d truck types:\n", num_truck_types);
    // for (int i = 0; i < num_truck_types; i++) {
    //     printf("TruckType %d: arrival=%d, length=%d, width=%d, height=%d, max_weight=%d, cost=%d\n",
    //            truck_types[i].id, truck_types[i].arrival_time, truck_types[i].length,
    //            truck_types[i].width, truck_types[i].height, truck_types[i].max_weight,
    //            truck_types[i].cost);
    // }
}

// Function to read supplier-truck relationships from file
void read_supplier_trucks(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    fscanf(file, "%*s %d", &num_supplier_trucks); // Read the number of entries
    char buffer[256];
    fgets(buffer, sizeof(buffer), file);
    fgets(buffer, sizeof(buffer), file); // Skip header line
    init_supplier_trucks(num_supplier_trucks);
    for (int i = 0; i < num_supplier_trucks; i++) {
        int truck_type_id, num_suppliers;
        fscanf(file, "%d %d", &truck_type_id, &num_suppliers);
        supplier_trucks[i].truck_type_id = truck_type_id;
        supplier_trucks[i].num_suppliers = num_suppliers;
        supplier_trucks[i].suppliers = (int *)malloc(num_suppliers * sizeof(int));
        if (!supplier_trucks[i].suppliers) {
            perror("Memory allocation failed for suppliers");
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < num_suppliers; j++) {
            fscanf(file, "%d", &supplier_trucks[i].suppliers[j]);
        }
    }
    fclose(file);

    // Debug print: list all supplier-truck relationships
    // printf("Read %d supplier-truck relationships:\n", num_supplier_trucks);
    // for (int i = 0; i < num_supplier_trucks; i++) {
    //     printf("SupplierTruck %d: truck_type=%d, num_suppliers=%d, suppliers=",
    //            i, supplier_trucks[i].truck_type_id, supplier_trucks[i].num_suppliers);
    //     for (int j = 0; j < supplier_trucks[i].num_suppliers; j++) {
    //         printf("%d ", supplier_trucks[i].suppliers[j]);
    //     }
    //     printf("\n");
    // }
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

// Function to initialize or resize the piles array in a truck
void ensure_piles_capacity(TruckInstance *truck, int required_capacity) {
    if (truck->piles_capacity >= required_capacity) {
        return;
    }
    int new_capacity = truck->piles_capacity == 0 ? 10 : truck->piles_capacity * 2;
    while (new_capacity < required_capacity) {
        new_capacity *= 2;
    }
    Pile *new_piles = (Pile *)realloc(truck->piles, new_capacity * sizeof(Pile));
    if (!new_piles) {
        perror("Memory allocation failed for piles");
        exit(EXIT_FAILURE);
    }
    truck->piles = new_piles;
    truck->piles_capacity = new_capacity;
}

// Function to initialize or resize the item_ids array in a pile
void ensure_pile_item_capacity(Pile *pile, int required_capacity) {
    if (pile->capacity >= required_capacity) {
        return;
    }
    int new_capacity = pile->capacity == 0 ? 10 : pile->capacity * 2;
    while (new_capacity < required_capacity) {
        new_capacity *= 2;
    }
    int *new_item_ids = (int *)realloc(pile->item_ids, new_capacity * sizeof(int));
    if (!new_item_ids) {
        perror("Memory allocation failed for pile item_ids");
        exit(EXIT_FAILURE);
    }
    pile->item_ids = new_item_ids;
    pile->capacity = new_capacity;
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
    ensure_pile_item_capacity(pile, pile->num_items + 1);
    pile->item_ids[pile->num_items] = item.id;
    pile->num_items++;
    pile->height += item.height;
    pile->weight += item.weight;
}

// Function to create a new pile in a truck
void create_new_pile(TruckInstance *truck, Item item, int x, int y) {
    ensure_piles_capacity(truck, truck->num_piles + 1);

    Pile new_pile;
    new_pile.x = x;
    new_pile.y = y;
    new_pile.length = item.length;
    new_pile.width = item.width;
    new_pile.height = item.height;
    new_pile.weight = item.weight;
    new_pile.stackability_code = item.stackability_code;
    new_pile.num_items = 1;
    new_pile.capacity = 10; // Initial capacity
    new_pile.item_ids = (int *)malloc(new_pile.capacity * sizeof(int));
    if (!new_pile.item_ids) {
        perror("Memory allocation failed for new pile's item_ids");
        exit(EXIT_FAILURE);
    }
    new_pile.item_ids[0] = item.id;

    truck->piles[truck->num_piles] = new_pile;
    truck->num_piles++;
    truck->current_weight += item.weight;
}

// Function to find a compatible truck type for an item
int find_compatible_truck_type(Item item) {
    for (int i = 0; i < num_truck_types; i++) {
        TruckType t = truck_types[i];
        // Check supplier compatibility
        if (!is_supplier_compatible(t.id, item.supplier)) {
            //printf("Truck type %d: incompatible supplier for item %d\n", i, item.id);
            continue;
        }
        // Check physical constraints
        if (t.max_weight < item.weight) {
            //printf("Truck type %d: insufficient weight capacity for item %d\n", i, item.id);
            continue;
        }
        if (t.length < item.length) {
            //printf("Truck type %d: insufficient length for item %d\n", i, item.id);
            continue;
        }
        if (t.width < item.width) {
            //printf("Truck type %d: insufficient width for item %d\n", i, item.id);
            continue;
        }
        if (t.height < item.height) {
            //printf("Truck type %d: insufficient height for item %d\n", i, item.id);
            continue;
        }
        // Check arrival time constraint
        if (item.earliest_arrival > t.arrival_time || item.latest_arrival < t.arrival_time) {
            //printf("Truck type %d: arrival time %d not within item's window [%d, %d]\n",i, t.arrival_time, item.earliest_arrival, item.latest_arrival);
            continue;
        }
        //printf("Truck type %d is compatible with item %d\n", i, item.id);
        return i;
    }
    //printf("No compatible truck type found for item %d\n", item.id);
    return -1; // No compatible truck type found
}

// Function to ensure trucks array has enough capacity
void ensure_trucks_capacity(int required_capacity) {
    if (trucks_capacity >= required_capacity) {
        return;
    }
    int new_capacity = trucks_capacity == 0 ? 10 : trucks_capacity * 2;
    while (new_capacity < required_capacity) {
        new_capacity *= 2;
    }
    TruckInstance *new_trucks = (TruckInstance *)realloc(trucks, new_capacity * sizeof(TruckInstance));
    if (!new_trucks) {
        perror("Memory allocation failed for trucks");
        exit(EXIT_FAILURE);
    }
    trucks = new_trucks;
    // Initialize new slots
    for (int i = trucks_capacity; i < new_capacity; i++) {
        trucks[i].piles = NULL;
        trucks[i].piles_capacity = 0;
        trucks[i].num_piles = 0;
    }
    trucks_capacity = new_capacity;
}

// Function to create a new truck instance
void create_new_truck(Item item) {
    int truck_type_id = find_compatible_truck_type(item);
    if (truck_type_id == -1) {
        //printf("No compatible truck type found for item %d\n", item.id);
        return; // Skip creating a new truck for this item
    }
    //printf("Creating new truck of type %d for item %d\n", truck_type_id, item.id);

    ensure_trucks_capacity(num_trucks + 1);

    TruckInstance new_truck;
    new_truck.truck_type_id = truck_type_id;
    new_truck.arrival_time = truck_types[truck_type_id].arrival_time;
    new_truck.current_weight = 0;
    new_truck.num_piles = 0;
    new_truck.piles_capacity = 0;
    new_truck.piles = NULL;

    trucks[num_trucks] = new_truck;

    // Find space for the new pile (which will contain only this item for now)
    int x, y;
    if (find_space_for_pile(&trucks[num_trucks], item.length, item.width, &x, &y)) {
        //printf("  Found space at (%d, %d) for item %d in new truck\n", x, y, item.id);
        create_new_pile(&trucks[num_trucks], item, x, y);
    } else {
        //printf("No space found for item %d in new truck\n", item.id);
    }
    num_trucks++;
}

// Function to free dynamically allocated memory for a pile
void free_pile(Pile *pile) {
    if (pile->item_ids) {
        free(pile->item_ids);
    }
}

// Function to free dynamically allocated memory for a truck
void free_truck(TruckInstance *truck) {
    for (int p = 0; p < truck->num_piles; p++) {
        free_pile(&truck->piles[p]);
    }
    if (truck->piles) {
        free(truck->piles);
    }
}

// Function to free all dynamically allocated memory
void cleanup() {
    if (items) free(items);
    if (truck_types) free(truck_types);
    if (supplier_trucks) {
        for (int i = 0; i < num_supplier_trucks; i++) {
            if (supplier_trucks[i].suppliers) {
                free(supplier_trucks[i].suppliers);
            }
        }
        free(supplier_trucks);
    }
    if (trucks) {
        for (int i = 0; i < num_trucks; i++) {
            free_truck(&trucks[i]);
        }
        free(trucks);
    }
}

// Main function to assign items to trucks
void assign_items_to_trucks() {
    for (int i = 0; i < num_items; i++) {
        Item item = items[i];
        int placed = 0;
        /*printf("Processing item %d (supplier=%d, weight=%d, dimensions=%dx%dx%d, arrival window=[%d, %d])\n",
               item.id, item.supplier, item.weight, item.length, item.width, item.height,
               item.earliest_arrival, item.latest_arrival);*/

        // Try to place in existing trucks
        for (int j = 0; j < num_trucks; j++) {
            TruckInstance *truck = &trucks[j];
            TruckType truck_type = truck_types[truck->truck_type_id];

            // Check if the truck can pick up from the item's supplier
            if (!is_supplier_compatible(truck_type.id, item.supplier)) {
                //printf("  Truck %d: incompatible supplier\n", j);
                continue;
            }

            // Check arrival time constraint
            if (item.earliest_arrival > truck_type.arrival_time ||
                item.latest_arrival < truck_type.arrival_time) {
                //printf("  Truck %d: incompatible arrival time (truck: %d, item: [%d, %d])\n", j, truck_type.arrival_time, item.earliest_arrival, item.latest_arrival);
                continue;
            }

            //printf("  Truck %d: compatible supplier and arrival time (arrival=%d)\n", j, truck_type.arrival_time);

            // Try to add to existing piles
            for (int p = 0; p < truck->num_piles; p++) {
                Pile *pile = &truck->piles[p];
                if (can_add_to_pile(pile, item, truck_type)) {
                    //printf("  Adding item %d to pile %d in truck %d\n", item.id, p, j);
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
                //printf("  Creating new pile in truck %d for item %d\n", j, item.id);
                create_new_pile(truck, item, x, y);
                placed = 1;
                break;
            } else {
                /*printf("  Could not create new pile in truck %d for item %d: weight=%d/%d, space=%d\n",
                       j, item.id, truck->current_weight + item.weight, truck_type.max_weight,
                       find_space_for_pile(truck, item.length, item.width, &x, &y));*/
            }
        }

        // If not placed yet, create a new truck
        if (!placed) {
            int truck_type_id = find_compatible_truck_type(item);
            if (truck_type_id == -1) {
                //printf("  No compatible truck type found for item %d\n", item.id);
                continue; // Skip this item if no compatible truck type is found
            }
            //printf("  Creating new truck for item %d\n", item.id);
            create_new_truck(item);
            placed = 1; // We assume create_new_truck succeeds
        }

        if (!placed) {
            //printf("  Could not place item %d in any truck\n", item.id);
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
    fprintf(file, "EQUIPE Grocamion\n");
    fprintf(file, "INSTANCE 2\n"); // Assuming instance 1 for now

    printf("Writing solution to %s\n", filename);
    printf("Number of trucks: %d\n", num_trucks);

    // Write truck and pile information
    for (int j = 0; j < num_trucks; j++) {
        /*printf("Writing truck %d (type %d) with %d piles\n",
               j, trucks[j].truck_type_id, trucks[j].num_piles);*/
        fprintf(file, "TYPECAMION %d\n", trucks[j].truck_type_id);
        for (int p = 0; p < trucks[j].num_piles; p++) {
            Pile pile = trucks[j].piles[p];
            /*printf("  Pile %d: length=%d, width=%d, x=%d, y=%d, num_items=%d\n",
                   p, pile.length, pile.width, pile.x, pile.y, pile.num_items);*/
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

    // Initialize trucks array with some initial capacity
    init_trucks(10); // Start with capacity for 10 trucks

    // Assign items to trucks
    assign_items_to_trucks();

    // Calculate and print total cost
    int total_cost = calculate_total_cost();
    printf("Total cost: %d\n", total_cost);

    // Write solution to file
    write_solution("res_2.txt");

    // Clean up dynamically allocated memory
    cleanup();

    return EXIT_SUCCESS;
}
