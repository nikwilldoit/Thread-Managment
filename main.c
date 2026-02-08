
#include "definitions.h"





int main(int argc,char *argv[]) {

    //Verify input validity
    if (argc != 3) {
        printf("Usage:\nProgram {number_of_customers} {seed}");
        exit(0);
    }
    int  number_of_customers = atoi(argv[1]);
    if (number_of_customers <= 0) {
        printf("Error, customers must be a positive number.");
            exit(-1);
    }
    unsigned int seed = atoi(argv[2]);
    if ( seed <= 0) {
        printf("Error, seed must be a positive number.");
            exit(-1);
    }
    initialize_random_numbers(seed, number_of_customers*12);

    setup_customer_service();

    //Initialize customer threads
    int rc,waiting_time;
    int *customer_ids = (int *) malloc(sizeof(int) * number_of_customers);
    pthread_t *customer_pthreads = (pthread_t*) malloc(sizeof(pthread_t) * number_of_customers );
    for (int i =0; i< number_of_customers; i++) {
        customer_ids[i] = i+1;
        rc = pthread_create(&customer_pthreads[i], NULL,customer_service, &customer_ids[i]);
        if (rc != 0) {
            printf("Error while creating threads. Terminating...");
            exit(-2);
        }
        waiting_time = get_random_number_between(T_orderlow,T_orderhigh);
        sleep(waiting_time);
    }

    for (int i  =0 ;i<number_of_customers;i++) {
        rc = pthread_join(customer_pthreads[i], NULL);
        if (rc !=0) {
            printf("Error. A problem has occured while trying to terminate the process");
            exit(-4);
        }
    }
    printf("\nΤέλος προσομείωσεις.\n");

    printf("\nΣτατιστικά παραγγελιών");
    printf("\nΣυνολικά Έσοδα: %d",Pizzaria.Revenue);
    printf("\nΠεπερόνι: %d", Pizzaria.pepperoni_pizza_sales);
    printf("\nΜαργαρίτα: %d", Pizzaria.margaritta_pizza_sales);
    printf("\nΣπέσιαλ: %d", Pizzaria.special_pizza_sales);
    printf("\nΠαραγγελίες που παραδώθηκαν: %d \nΠαραγγελίες που ακυρώθηκαν: %d",Pizzaria.successfull_orders,number_of_customers- Pizzaria.successfull_orders);
    printf("\n\nΧρόνος για την παράδοση της πίτσας.");
    printf("\nΜέγιστος χρόνος κρυόματος της πίτσας: %d", Pizzaria.max_cooling_time);
    int average_cooling_time = Pizzaria.total_pizza_cooling_time/Pizzaria.successfull_orders;
    printf("\nΜέσος χρόνος κρυόματος της πίτσας: %d",average_cooling_time);

    printf("\n\nΣτατιστικά εξυπηρέτησης πελατών:");
    printf("\nΜέγιστος Χρόνος εξυπηρέτησης πελάτη: %d", Pizzaria.max_customer_service_time);
    int average_service_time = Pizzaria.total_customer_service_time/Pizzaria.successfull_orders;
    printf("\nΜέσος Χρόνος εξυπηρέτησης πελάτη: %d",average_service_time);

    destroy_random_numbers();
    close_Pizzaria();

    return 0;
}

/*
 *
 *  All the jjobs handlement
 *
 */

void setup_customer_service() {
    Pizzaria.available_phones = N_tel;
    Pizzaria.prep_cook = N_cook;
    Pizzaria.ovens = N_oven;
    Pizzaria.deliverers = N_deliver;
    Pizzaria.packers = N_packer;
    Pizzaria.Revenue =0;
    Pizzaria.pepperoni_pizza_sales =0;
    Pizzaria.margaritta_pizza_sales =0;
    Pizzaria.special_pizza_sales =0;
    Pizzaria.successfull_orders =0;
    Pizzaria.total_customer_service_time =0;
    Pizzaria.max_customer_service_time = 0;
    Pizzaria.total_pizza_cooling_time = 0;
    Pizzaria.max_cooling_time =0;




    pthread_mutex_init(&Pizzaria.telephone_mutex,NULL);
    pthread_mutex_init(&Pizzaria.write_on_terminal,NULL);
    pthread_mutex_init(&Pizzaria.revenue_mutex,NULL);
    pthread_mutex_init(&Pizzaria.prep_cook_mutex,NULL);
    pthread_mutex_init(&Pizzaria.oven_mutex,NULL);
    pthread_mutex_init(&Pizzaria.packing_mutex,NULL);
    pthread_mutex_init(&Pizzaria.delivery_mutex,NULL);
    pthread_mutex_init(&Pizzaria.pizza_delivery_statistics_mutex,NULL);

    pthread_cond_init(&Pizzaria.answer_telephone,NULL);
    pthread_cond_init(&Pizzaria.look_for_cook,NULL);
    pthread_cond_init(&Pizzaria.check_for_ovens,NULL);
    pthread_cond_init(&Pizzaria.available_packer,NULL);
    pthread_cond_init(&Pizzaria.make_delivery,NULL);
}

void close_Pizzaria() {
    pthread_mutex_destroy(&Pizzaria.telephone_mutex);
    pthread_mutex_destroy(&Pizzaria.write_on_terminal);
    pthread_mutex_destroy(&Pizzaria.revenue_mutex);
    pthread_mutex_destroy(&Pizzaria.prep_cook_mutex);
    pthread_mutex_destroy(&Pizzaria.oven_mutex);
    pthread_mutex_destroy(&Pizzaria.packing_mutex);
    pthread_mutex_destroy(&Pizzaria.delivery_mutex);
    pthread_mutex_destroy(&Pizzaria.pizza_delivery_statistics_mutex);

    pthread_cond_destroy(&Pizzaria.telephone_mutex);
    pthread_cond_destroy(&Pizzaria.look_for_cook);
    pthread_cond_destroy(&Pizzaria.check_for_ovens);
    pthread_cond_destroy(&Pizzaria.available_packer);
    pthread_cond_destroy(&Pizzaria.make_delivery);

}





void *customer_service(void *id) {
    //extract id
    int * orderId = (int*) id;
    struct timespec previous , current;
    clock_gettime(CLOCK_REALTIME,&previous);

    int process_time;

    //reserve a call
    pthread_mutex_lock(&Pizzaria.telephone_mutex);
    while (1) {
        if (Pizzaria.available_phones) {
            Pizzaria.available_phones--;
            break;
        }else {
            pthread_cond_wait(&Pizzaria.answer_telephone, &Pizzaria.telephone_mutex );
        }
    }
    //Phone secured, close mutex
    pthread_mutex_unlock(&Pizzaria.telephone_mutex);

    //Take order

    int pizza_count = get_random_number_between(N_orderlow,N_orderhigh);
    //count of pizzas
    int margaritas = 0;
    int peperonis =0 ;
    int specials =0;
    int odds_per_pizza;
    for (int i = 0; i< pizza_count; i++) {
        odds_per_pizza = get_random_number_between(1,100);
        if (odds_per_pizza <= P_p*100) {
            peperonis++;
        } else if (odds_per_pizza <= P_p*100 +P_m*100) {
            margaritas++;
        }else {
            specials++;
        }
    }

    //Process Order Time
    process_time = get_random_number_between(T_paymentlow,T_paymenthigh);
    sleep(process_time);



    int odds_for_payment_to_fail = get_random_number_between(1,100);
    if (P_fail*100 >= odds_for_payment_to_fail) {
        pthread_mutex_lock(&Pizzaria.write_on_terminal);
        printf("\nΗ παραγγελία με αριθμό %d απέτυχε.", *orderId);
        pthread_mutex_unlock(&Pizzaria.write_on_terminal);
        //release phones
        pthread_mutex_lock(&Pizzaria.telephone_mutex);
        Pizzaria.available_phones++;
        pthread_mutex_unlock(&Pizzaria.telephone_mutex);
        pthread_cond_signal(&Pizzaria.answer_telephone);
        pthread_exit(id);

    }



    pthread_mutex_lock(&Pizzaria.write_on_terminal);
    printf("\nΗ παραγγελία με αριθμό %d καταχωρήθηκε.", *orderId);
    pthread_mutex_unlock(&Pizzaria.write_on_terminal);

    //release phone
    pthread_mutex_lock(&Pizzaria.telephone_mutex);
    Pizzaria.available_phones++;
    pthread_mutex_unlock(&Pizzaria.telephone_mutex);

    pthread_cond_signal(&Pizzaria.answer_telephone);


    //Update profits and statistical data
    pthread_mutex_lock(&Pizzaria.revenue_mutex);
    Pizzaria.Revenue += margaritas * C_m + peperonis*C_p + specials * C_s;
    Pizzaria.margaritta_pizza_sales += margaritas;
    Pizzaria.pepperoni_pizza_sales +=peperonis;
    Pizzaria.special_pizza_sales +=specials;
    Pizzaria.successfull_orders++;
    pthread_mutex_unlock(&Pizzaria.revenue_mutex);





    //Pick a cook
    pthread_mutex_lock(&Pizzaria.prep_cook_mutex);
    while (1) {
        if (Pizzaria.prep_cook) {
            Pizzaria.prep_cook--;
            break;
        } else {
            pthread_cond_wait(&Pizzaria.look_for_cook,&Pizzaria.prep_cook_mutex);
        }
    }
    pthread_mutex_unlock(&Pizzaria.prep_cook_mutex);
    process_time  = T_prep * pizza_count;
    sleep(process_time);

    //Reserve pizza ovens
    //lock to try to get ovens
    pthread_mutex_lock(&Pizzaria.oven_mutex);
    while (1) {

        if (Pizzaria.ovens >= pizza_count) {
            //Take them
            Pizzaria.ovens -= pizza_count;
            break;
        } else  {
            //Pause processing we are wasting time
            pthread_cond_wait(&Pizzaria.check_for_ovens,&Pizzaria.oven_mutex);
        }
    }
    pthread_mutex_unlock(&Pizzaria.oven_mutex);
    //Furnaces reserved successfully

    //let the cook back
    pthread_mutex_lock(&Pizzaria.prep_cook_mutex);
    Pizzaria.prep_cook++;
    pthread_mutex_unlock(&Pizzaria.prep_cook_mutex);
    pthread_cond_signal(&Pizzaria.look_for_cook);
    //Bake the furnace
    sleep(T_bake);
    pthread_mutex_lock(&Pizzaria.packing_mutex);

    while (1) {
        if (Pizzaria.packers > 0) {
            Pizzaria.packers--;
            break;
        } else {
            pthread_cond_wait(&Pizzaria.available_packer,&Pizzaria.packing_mutex);
        }
    }
    pthread_mutex_unlock(&Pizzaria.packing_mutex);

    //timmme to pack  the pizzaas
    process_time = T_pack * pizza_count;
    sleep(process_time);

    //start counting time for freezing
    struct timespec pizza_cooked_timestamp;
    clock_gettime(CLOCK_REALTIME,&pizza_cooked_timestamp);


    //release the oven
    pthread_mutex_lock(&Pizzaria.oven_mutex);
    Pizzaria.ovens += pizza_count;
    pthread_cond_broadcast(&Pizzaria.check_for_ovens);
    pthread_mutex_unlock(&Pizzaria.oven_mutex);

    //release the packers
    pthread_mutex_lock(&Pizzaria.packing_mutex);
    Pizzaria.packers++;
    pthread_cond_signal(&Pizzaria.available_packer);
    pthread_mutex_unlock(&Pizzaria.packing_mutex);

    //get current time
    clock_gettime(CLOCK_REALTIME,&current);
    int packaging_time = current.tv_sec - previous.tv_sec;
    pthread_mutex_lock(&Pizzaria.write_on_terminal);
    printf("\nΗ παραγγελια με αριθμό %d ετοιμάστηκε σε %d λεπτά.",  *orderId,packaging_time);
    pthread_mutex_unlock(&Pizzaria.write_on_terminal);

    //delivery time
    pthread_mutex_lock(&Pizzaria.delivery_mutex);
    while (1) {
        if (Pizzaria.deliverers >0) {
            Pizzaria.deliverers--;
            break;
        } else {
            pthread_cond_wait(&Pizzaria.make_delivery,&Pizzaria.delivery_mutex);
        }
    }
    pthread_mutex_unlock(&Pizzaria.delivery_mutex);
    process_time = get_random_number_between(T_dellow,T_delhigh);
    sleep(process_time);

    clock_gettime(CLOCK_REALTIME,&current);
    //add statistics
    int total_time_for_delivery = current.tv_sec - previous.tv_sec;
    int cooling_time = current.tv_sec - pizza_cooked_timestamp.tv_sec;;
    pthread_mutex_lock(&Pizzaria.pizza_delivery_statistics_mutex);
    //delivery time
    Pizzaria.total_customer_service_time += total_time_for_delivery;
    if (total_time_for_delivery > Pizzaria.max_customer_service_time) {
        Pizzaria.max_customer_service_time = total_time_for_delivery;
    }
    //cooling pizza statistics
    Pizzaria.total_pizza_cooling_time +=cooling_time;
    if (cooling_time > Pizzaria.max_cooling_time) {
        Pizzaria.max_cooling_time = cooling_time;
    }
    pthread_mutex_unlock(&Pizzaria.pizza_delivery_statistics_mutex);

    int delivery_time = current.tv_sec- previous.tv_sec;
    pthread_mutex_lock(&Pizzaria.write_on_terminal);
    printf("\nΗ παραγγελία με αριθμό %d παραδόθηκε σε %d λεπτά.",*orderId ,delivery_time );
    pthread_mutex_unlock(&Pizzaria.write_on_terminal);



    sleep(process_time);

    pthread_mutex_lock(&Pizzaria.delivery_mutex);
    Pizzaria.deliverers++;
    pthread_cond_signal(&Pizzaria.make_delivery);

    pthread_mutex_unlock(&Pizzaria.delivery_mutex);

    pthread_exit(id);


}


/*
 *
 *Random numbers handler
 *
 *
 */


void initialize_random_numbers(unsigned int seed,int numbers_to_make) {
    //start with the mutex
    pthread_mutex_init(&random_numbers_lock, NULL);

    //load the random numbers and initialize them
    random_numbers = (int*) malloc(numbers_to_make * sizeof(int));
    for (int i =0; i < numbers_to_make;i++) {
        random_numbers[i] = rand_r(&seed);
    }
}

int get_random_number_between(int start,int end) {
    //lock to extract
    pthread_mutex_lock(&random_numbers_lock);
    //extract and increment
    int to_extract = random_numbers[random_numbers_number_to_be_used];
    random_numbers_number_to_be_used++;
    pthread_mutex_unlock(&random_numbers_lock);
    //modify to fit and return
    to_extract %= end - start;
    to_extract += start;
    return to_extract;
}

void destroy_random_numbers() {
    free(random_numbers);
}

