#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Define a struct for goods
typedef struct {
    int id;
    int supplier_code;
    int length;
    int width;
    int height;
    int weight;
    int stackability_code;
    int earliest_arrival_time;
    int latest_arrival_time;
    int inventory_cost;
} Good;

// Define a struct for trucks
typedef struct {
    int id;
    int arrival_time;
    int length;
    int width;
    int height;
    int max_weight;
    int cost;
    int* supplier_codes;
    int num_suppliers;
} Truck;


// Define a struct for stacks
typedef struct {
    int x;
    int y;
    int* goods_ids;
    int num_goods;
    int max_goods;
} Stack;

// Define a struct for trucks with stacks
typedef struct {
    Truck truck;
    Stack* stacks;
    int num_stacks;
    int max_stacks;
} PlannedTruck;

// Function to read goods from a file
Good* read_goods(const char* filename, int* num_goods) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "nbObjets %d", num_goods);
    Good* goods = (Good*)malloc(*num_goods * sizeof(Good));

    // Skip the header line
    char header[256];
    fgets(header, sizeof(header), file);

    for (int i = 0; i < *num_goods; i++) {
        fscanf(file, "%d %d %d %d %d %d %d %d %d %d",
               &goods[i].id, &goods[i].supplier_code, &goods[i].length,
               &goods[i].width, &goods[i].height, &goods[i].weight,
               &goods[i].stackability_code, &goods[i].earliest_arrival_time,
               &goods[i].latest_arrival_time, &goods[i].inventory_cost);
    }

    fclose(file);
    return goods;
}

// Function to read trucks from a file
Truck* read_trucks(const char* filename, int* num_trucks) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "nbCamions %d", num_trucks);
    Truck* trucks = (Truck*)malloc(*num_trucks * sizeof(Truck));

    // Skip the header line
    char header[256];
    fgets(header, sizeof(header), file);

    for (int i = 0; i < *num_trucks; i++) {
        int num_suppliers;
        fscanf(file, "%d %d %d %d %d %d %d %d",
               &trucks[i].id, &trucks[i].arrival_time, &trucks[i].length,
               &trucks[i].width, &trucks[i].height, &trucks[i].max_weight,
               &trucks[i].cost, &num_suppliers);

        trucks[i].num_suppliers = num_suppliers;
        trucks[i].supplier_codes = (int*)malloc(num_suppliers * sizeof(int));
        for (int j = 0; j < num_suppliers; j++) {
            fscanf(file, "%d", &trucks[i].supplier_codes[j]);
        }
    }

    fclose(file);
    return trucks;
}


// Function to initialize a planned truck
PlannedTruck init_planned_truck(Truck truck) {
    PlannedTruck planned_truck;
    planned_truck.truck = truck;
    planned_truck.max_stacks = truck.length * truck.width; // Maximum possible stacks
    planned_truck.stacks = (Stack*)malloc(planned_truck.max_stacks * sizeof(Stack));
    planned_truck.num_stacks = 0;
    return planned_truck;
}

// Function to check if a good can be added to a stack
bool can_add_to_stack(Stack* stack, Good* good, PlannedTruck* planned_truck) {
    // Check if the stack has the same stackability code and if there is space
    // This is a simplified check; you'll need to implement the full logic
    return true;
}

// Function to add a good to a stack
void add_to_stack(Stack* stack, int good_id) {
    stack->goods_ids[stack->num_goods++] = good_id;
}

// Function to try inserting a good into existing stacks
bool insert_into_existing_stacks(Good* good, PlannedTruck* planned_trucks, int num_planned_trucks) {
    for (int i = 0; i < num_planned_trucks; i++) {
        for (int j = 0; j < planned_trucks[i].num_stacks; j++) {
            if (can_add_to_stack(&planned_trucks[i].stacks[j], good, &planned_trucks[i])) {
                add_to_stack(&planned_trucks[i].stacks[j], good->id);
                return true;
            }
        }
    }
    return false;
}

// Main function to plan the goods into trucks
void plan_goods_to_trucks(Good* goods, int num_goods, Truck* trucks, int num_trucks) {
    PlannedTruck* planned_trucks = (PlannedTruck*)malloc(num_trucks * sizeof(PlannedTruck));
    int num_planned_trucks = 0;

    for (int i = 0; i < num_goods; i++) {
        if (!insert_into_existing_stacks(&goods[i], planned_trucks, num_planned_trucks)) {
            // If insertion into existing stacks fails, create a new stack or plan a new truck
            // This part is more complex and needs to be implemented
        }
    }

    // Free allocated memory
    for (int i = 0; i < num_planned_trucks; i++) {
        for (int j = 0; j < planned_trucks[i].num_stacks; j++) {
            free(planned_trucks[i].stacks[j].goods_ids);
        }
        free(planned_trucks[i].stacks);
    }
    free(planned_trucks);
}

int main() {
    int num_goods;
    Good* goods = read_goods("Instances/I1_input_items.txt", &num_goods);

    int num_trucks;
    Truck* trucks = read_trucks("Instances/I1_input_trucks.txt", &num_trucks);

    plan_goods_to_trucks(goods, num_goods, trucks, num_trucks);

    // Free allocated memory
    free(goods);
    for (int i = 0; i < num_trucks; i++) {
        free(trucks[i].supplier_codes);
    }
    free(trucks);

    return 0;
}


