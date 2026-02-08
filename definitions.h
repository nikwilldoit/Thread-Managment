#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

//Employees on each section

#define N_tel 2
#define N_cook 2
#define N_oven 10
#define N_packer 2
#define N_deliver 10

/* The following data involves the telephone orders*/

//Time to pick the order
#define T_orderlow 1
#define T_orderhigh 5

//Pizzas to order
#define N_orderlow 1
#define N_orderhigh 5


//Likelyhood to order specific pizza
#define P_m 0.45f
#define P_p 0.35f
#define P_s 0.2f

//Time to process order payment
#define T_paymentlow 1
#define T_paymenthigh 3

#define P_fail 0.05f

//Pizza cost
#define C_m 12
#define C_p 14
#define C_s 15

//Cook time
#define T_prep 1
#define T_bake 10
#define T_pack 1

//Delivery time
#define T_dellow 10
#define T_delhigh 15


/*
 *Customer handling system
 *
 */
void setup_customer_service() ;

struct Pizzaria_Structure {
    //variables
    int available_phones;
    int prep_cook;
    int ovens;
    int packers;
    int deliverers;
    //Mutexes
    pthread_mutex_t telephone_mutex;
    pthread_mutex_t write_on_terminal;
    pthread_mutex_t revenue_mutex;
    pthread_mutex_t prep_cook_mutex;
    pthread_mutex_t oven_mutex;
    pthread_mutex_t packing_mutex;
    pthread_mutex_t delivery_mutex;
    pthread_mutex_t pizza_delivery_statistics_mutex;
    //Conditionals
    pthread_cond_t answer_telephone;
    pthread_cond_t look_for_cook;
    pthread_cond_t check_for_ovens;
    pthread_cond_t available_packer;
    pthread_cond_t make_delivery;
    //Statistics
    int pepperoni_pizza_sales;
    int margaritta_pizza_sales;
    int special_pizza_sales;
    int successfull_orders;
    int total_customer_service_time;
    int max_customer_service_time;
    int total_pizza_cooling_time;
    int max_cooling_time;
    int Revenue;

    //Special mutex


} Pizzaria;

void *customer_service(void *id);


/*
 *Random numbers declerations
 */

int random_numbers_number_to_be_used = 0;
pthread_mutex_t random_numbers_lock;
int *random_numbers;
void initialize_random_numbers(unsigned int seed,int numbers_to_make);
int get_random_number_between(int start,int end);
