#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

/*Main Constants (arrays) For Car Sales*/
/*creating file within the project folder
 *this file is in the build cache
 *adding './' to sales_data.csv moves it to the current working directory the '.' means current working directory
 #define SALES_FILE "./sales_data.csv" as an example - previous data loads
 with './' it prevents Clion creating a random working directory
 "sales_data.csv" here the c program changes the directory each time it ran resetting all data for testing*/
#define SALES_FILE "./sales_data.csv"  //so previous saves load correctly - for reliability
//Files Variables and constants
#define FILE_OPENED 0
#define FILE_CLOSED 1
#define FILE_ERROR 2 //error handling
/*FILE is a big structure with info
 * FILE * is address/handle/remote control for the file - using a pointer to handle the file
 * NULL means pointer currently points to nothing this allows all the functions that have file to use it,
 * like openfile, loadSalesFromFile, to check if(file!=NULL){...}
 */
FILE *file = NULL; //global file pointer
unsigned char fileStatus = FILE_CLOSED;
#define MAX_CARS 9 //capacity how many cars could fit in my array
#define DISCOUNT_MULTIBUY_PERCENTAGE 0.15f
#define DISCOUNT_MULTIBUY_AMOUNT 2
#define DISCOUNT_MEMBER_PERCENTAGE 0.25f
//I am only using 5 spaces out of 9, with each space being able to hold 20 characters.
char carModels[MAX_CARS][20] = {"Ford Mustang", "Chevrolet Camaro", "Dodge Charger", "BMW 3 Series", "Audi A4"};
float carPrices[MAX_CARS] = {55000.0f, 60000.0f, 58000.0f, 45000.0f, 47000.0f};
unsigned short years [MAX_CARS] = {2020, 2019, 2021, 2018, 2022};
int carStock[MAX_CARS] = {4, 5, 3, 6, 3};
unsigned short n_models = 5; // how many spaces I used
int CarsSold = 0;
/* Below Represents one sale made by a customer
 * creating a custom data type called Sale
 *without this struct I would need to create 8 separate arrays which would be more confusing when debugging
 */
typedef struct {
    char customerName[50];
    int customerAge;
    int carIndex;
    unsigned short quantity;
    float totalPrice;
    int discountApplied; // 0 = no, 1 = yes
    float discountPercent;
    time_t purchaseTime; // timestamp of purchase

} Sale;
/*To Store feedback from a customer */
typedef struct {
    int carIndex;
    int rating; // Rating out of 5
    char comment[100]; //short comment from customer
} Feedback;
#define MAX_SALES 12 //stores up to 12 sales
#define MAX_FEEDBACK 10
Sale sales[MAX_SALES]; //data the user enters is stored here using the sales struct
int salesCount = 0; //no sales stored - this acts as a counter how many sale slots are filled - index manager
Feedback feedbacks[MAX_FEEDBACK]; // sorting all feedback entries
int feedbackCount = 0; // how many feedback entries are there (sorted)
/*Now the program can have a history of sales not just the totals with each sale linking back to carModels[sales[i].carIndex]
 * and carPrices[...] allowing for summed totals and add feedback linking it back to specific models as well*/
    /* Clear Screen to make it easier for useer to read*/
#ifdef _WIN32
#define CLEAR_SCREEN "cls" //Clear command on Windows
#else
#define CLEAR_SCREEN "clear" //Clear command on MAC_OS and Linux
#endif
    /* Menu Options Constants */
#define MENU_BUY_CAR 'A'
#define MENU_VIEW_SALES 'B'
#define MENU_VIEW_SALES_TARGET 'C'
#define MENU_VIEW_AVAILABLE_CARS 'D'
#define MENU_FEEDBACK 'E'
#define MENU_EXIT 'X'
#define TRUE 1
#define FALSE 0

FILE *createFile(char *fileName) {
    file = fopen(fileName, "w"); /*try and create file 'w'- open file for writing or to empty an already existing file
    if file already exists 'Truncate (empty)' it fopen() handles this so empties the file firs then writes the new sales*/
    if (file != NULL) {
        fclose(file); /*close after making as it only ensures that the file exists
        why close it ? so that another function can safely open the file in 'r,w or a' NULL is used here to
        safely handle any errors creating the file
        any error such as disk error or permission denied
        this function doesn't run after file is created as once file exists 'r' open sales code executes */
    }
    return file; //returns the pointer after a successful creation of a file
}
void openFile(char *fileName, char *mode) {
    file = fopen(fileName, mode); //connected with openFile(SALES_FILE "r"); reads file without changing it
    if (file == NULL) {
        //Try to create file first if file didnt exist
        if (createFile(fileName) == NULL) {
            fileStatus = FILE_ERROR;
        } else {
            //now that its there, try opening again
            file = fopen (fileName, mode);
            if (file == NULL) {
                fileStatus = FILE_ERROR;
            }else {
                fileStatus = FILE_OPENED;
            }
        }
    }else {
    fileStatus = FILE_OPENED;
    }
}

//now if the file has been created and is open - close it!
void closeFile(void) {
    if (fileStatus == FILE_OPENED && file != NULL) {
        fclose(file);
        file = NULL;
        fileStatus = FILE_CLOSED;
    }
}
void clearScreen() {
    system(CLEAR_SCREEN); // Clear the console screen as this expands to cls or clear based on OS
}
void pauseProgram(char userChoice) {
    if (userChoice == MENU_EXIT) {
        printf("\n\nPress Enter to Exit...");
    }
    else {
        printf("\n\nPress Enter to return to the Menu...");
    }
    getchar(); //clears the left over new line from the previous scanf function when user presses <ENTER>
    getchar(); //now waits for the user, without this the code with skip to the menu
}
void menu_greeting() {
    printf("Welcome to Jasper's Car Sales office website!\n\n");
    printf("This Site May Seem Amateur But The Deals Are Real-ish!\n\n");
    printf("Car_Sales_Menu\n");
}
void menu_options() {
    printf("These Are Your Options Please Select One:\n");
    printf(" %c Buy Car\n", MENU_BUY_CAR);
    printf(" %c View Sales\n", MENU_VIEW_SALES);
    printf(" %c View Sales Targets\n", MENU_VIEW_SALES_TARGET);
    printf(" %c View Available Cars\n", MENU_VIEW_AVAILABLE_CARS);
    printf(" %c View Feedback From Loyal Customers\n", MENU_FEEDBACK);
    printf(" %c Exit\n\n", MENU_EXIT);
}
/*the function below allows me to call it when I need to capture a single character
 * like Y/N later in the code and flushes the input buffer without me duplicating code unscesarily
 */
char getCharFromConsole(const char message[203]) {
    char userInput;
    // prompt user using the value of the given message
    printf("%s", message);
    // capture the character
    scanf(" %c", &userInput);

    while (getchar() != '\n'); // clear input buffer - left over input <enter> character so that the program works smoothly

    // return the captured value
    return userInput;
}
unsigned short getUnsignedShortFromConsole( const char message[203]) {
    // will be used to store the captured number
    unsigned short userInput;

    do {
        // prompt user using the value of the given message
        printf("%s", message);
        /* so the Int_Dev_Env is recommending I use strtoul as scanf doesn't detect all conversion errors
         * but for this assignment I'll keep it simple and use scanf with a loop to re-prompt on error checking
         * the return value of scanf and flushes the buffer after each attempt
         */
        int scanResult = scanf(" %hu", &userInput);
        while (getchar() != '\n'); // clear input buffer

        if (scanResult == 1) {
            return userInput;
        }
        printf("Invalid Number Please Try Again.\n");
    } while (1);
        // finally, return/export the value so it can be used
        // by whatever statement called this Function
        return userInput;

    /*Keeps asking until valid input*/
}
//To be used by qsort later- qsort calls this a few times to figure out the order
int compareYears(const void *a, const void *b) {
    int indexA = *(const int *)a; //convert pointer to int value
    int indexB = *(const int *)b;
    /*When qsort compares two items it gives it
     * pointer item to A and same with B - take the pointer and treat it like an integer and store it there
     * the next function below then as an example takes indexA = 3 and indexB = 1 and subtracts
     * if result > 0 B should come before A (2022 - 2018 = 4 is positive so B(newer year goes first))
     * indexList = {0,1,2,3,4}
     */
    return years[indexB] - years[indexA]; //bigger (newer) year comes first
}

char getMenuchoice(const char *message) {
    char c = getCharFromConsole(message); //c = 'b'
    return toupper(c); //returns B
}
void buildSortedIndexListByYear(int indexList[]) {
    for (int i = 0; i < n_models; i++) {
        indexList[i] = i; //start with 0,1,2 ... looping through the array
    }

    /*sort the index list based on year using qsort and compareYears()
     * take indexList[0,1,2...] - n_models (5) - sizeof(int) shuffle integers of 4 bytes each as the index num are int
     * - compare the years which then looks into the varriable and organises the years accordingly
     */
    qsort(indexList, n_models, sizeof(int), compareYears);
}
void displayAvailableCars() {
    printf("Available Cars (sorted by year in descending order):\n");
    int indexList[MAX_CARS];
    buildSortedIndexListByYear(indexList); //working with qsort
    //fill the indexList with 0,1,2,3... n_models-1
    for (int i = 0; i < n_models; i++) {
        int idx = indexList[i];
        printf("%d. %s (%d) - $%.2f - Stock: %d\n",
                i + 1,          //menu number
                carModels[idx],
                years[idx],
                carPrices[idx],
                carStock[idx]);
    }
}

float calculateFinalPriceWithDiscount(int carIndex, //which car model was chosen (index number)
    unsigned short quantity, //how many cars
    int *discountApplied,  //out: 0 or 1 if discount applied
    float *discountPercent /* Pointer to return more than one output */ )
{
    float basePrice = carPrices[carIndex] * quantity; // base cost with no discount yet - quantity is how much they have bought
    float discount = 0.0f;    //start with zero discount
    *discountApplied = FALSE; //assume no discount
    //ask if they are a member and this then points to the function created earlier to decide to offer discount or not
    char MemberAnswer = getCharFromConsole
    ("Are you a member of Jasper's fantastic Car Sales Loyalty Program? The hope is yes (Y/N): ");

    //decide which discount to apply
    if (MemberAnswer == 'Y' || MemberAnswer == 'y') {
        discount += DISCOUNT_MEMBER_PERCENTAGE; //25% discount
        *discountApplied = TRUE;
} else if (quantity >= DISCOUNT_MULTIBUY_AMOUNT) {
    discount += DISCOUNT_MULTIBUY_PERCENTAGE; //15% discount
    *discountApplied = TRUE; /* the (*) is because we are dereferencing the pointer to set the value at that address
    like going into a house and changing what's inside as an example
    its like saying take this pointer and store a value there it lets a function return extra outputs besides return a value
    this prevents creating multiple return values and allows pointer to change its local copy */
}
*discountPercent = discount; //return the new total discount percentage to user
    float finalPrice = basePrice;
    if (*discountApplied) {
        finalPrice = basePrice * (1.0f - discount); //for example 30,000 * (1 - 0.15) = 25500.0f
        printf("\n A discount of %.2f%% has been applied to your purchase!\n", discount * 100.0f);
    }else {
        printf("No Discount Applied.\n");
    }
    return finalPrice;
}
//function below ensures the values 1-5 are entered rather than any number or text
unsigned short getUnsignedShortInRange( const char message[203], //prompt text
    unsigned short minValue, //smallest allowed number entered by programmer
    unsigned short maxValue  //largest allowed number entered by programmer
    ) {
    unsigned short value;

    while (1) {
        value = getUnsignedShortFromConsole(message);
        if (value >= minValue && value <= maxValue) {
            return value; //valid input
        }
        printf(" Invalid Input Please Enter a Number Between %hu and %hu\n", minValue, maxValue); //ensures that 1-5 is entered
    }
}

void askForFeedback(int carIndex) {
    if (feedbackCount >= MAX_FEEDBACK) {
        printf("\n Oops Feedback storage is full cannot record more feedback. Sorry!\n");
        return;
    }
    char answer = getCharFromConsole("\n Would you like to provide valuable feedback for your purchase? (Y/N): ");
    if (answer != 'Y' && answer != 'y') {
        printf("No feedback will be recorded.\n");
        return; //user said no
    }
    unsigned short rating = getUnsignedShortInRange
    ("Please rate your car (1-5) 1 best worst and 5 being Knock your socks off amazing: ", 1, 5);
    char commentBuffer[100];
    printf(" Please enter any thoughts about your car purchase and experience: ");
    scanf(" %99[^\n]", commentBuffer); //read up to 99 chars + null terminator
    while (getchar() != '\n'); //clear input buffer (left over new line)

    //store feedback into array
    feedbacks[feedbackCount].carIndex = carIndex;
    feedbacks[feedbackCount].rating = (int)rating;
    /*comment buffer is a temp note pad where user types input
     * feedbacks[] etc is a permanent storage box in feedback history
     * strcpy is used to copy the user input in this array
     */
    strcpy(feedbacks[feedbackCount].comment, commentBuffer);
    feedbackCount++; //move to next free slot in the array

    printf(" Many Thanks Indeed, this feedback will be taken with not buts or maybes!\n");
    printf(" Have  a great week!\n");
}

void menu_buyCar() {
    // Function to handle car purchase
    printf(" You Have Chosen Buy a Car:\n");
    printf("\nPress Enter to Continue...\n");
    getchar();

    //check stock
    int totalstock = 0;
    for (int i = 0; i < n_models; i++) {
        totalstock += carStock[i];
    }
    if (totalstock <= 0) {
        printf("Sorry There are no more cars available.\n");
        return;
    }
    //build sorted index List and show cars in that order
    int sortedIndex[MAX_CARS];
    buildSortedIndexListByYear(sortedIndex);

    printf(" Available Cars ( Sorted By Year In descending Order ): \n");
    for (int i = 0; i < n_models; i++) {
        int idx = sortedIndex[i];
        printf(" %d. %s ( %d ) - $%.2f - Stock: %d\n",
            i + 1,
            carModels[idx], //gets the name of the model
            years[idx],
            carPrices[idx],
            carStock[idx]); //stock count
    }

    //ask which model they want to buy
    unsigned short carChoice;
    int carIndex;

    while (1){
        carChoice = getUnsignedShortInRange(" Enter your choice of car you want to buy ( Please select 1-5 ): ",
            1, n_models); // guarantees 1 <= carChoice <= n_models
        //convert menu choice (1...n_models) into real index via sortedIndex[]
        carIndex = sortedIndex[carChoice - 1]; // convert to base 0 in the index

        //check if model is in stock
        if (carStock[carIndex] > 0) {
                break;
        }
        printf(" Sorry That Model is currently Out of Stock due to Popular Demand Please Choose Another One.\n");
    }
    //ask how many of that model they want to buy, ensure there is enough stock
    unsigned short quantity;
    while (1) {
        quantity = getUnsignedShortFromConsole(" Enter the quantity of this type of car that you want to buy: ");
        if (quantity == 0 ) {
            printf(" You must at-least One Car to proceed with purchase.\n");
        } else if (quantity > carStock[carIndex]) {
            printf(" Sorry we only have %d of this model in stock. Please enter a lower quantity.\n", carStock[carIndex]);
        }else{
            break; //valid quantity
        }
    }
    char nameBuffer[51];
    printf(" Now Please Enter your name: ");
    scanf(" %50[^\n]", nameBuffer); // read the full name including spaces
    while (getchar() != '\n'); // clear input buffer - left over newline.

    //ask for age
    unsigned short age = getUnsignedShortFromConsole(" Finally To Ensure This Purchase Please Enter Your Age: ");

    //calculate discount using helper function

    float finalPrice;
    int discountApplied;
    float discountPercent;

    finalPrice = calculateFinalPriceWithDiscount(carIndex,
        quantity, &discountApplied,
        &discountPercent
        );

    //store the sale in the sales array if there is space
    if (salesCount < MAX_SALES) {
        sales[salesCount].carIndex = carIndex;
        sales[salesCount].quantity = quantity;
        sales[salesCount].totalPrice = finalPrice;
        sales[salesCount].customerAge = age;

        // RECORD DISCOUNT INFO
        sales[salesCount].discountApplied = discountApplied;
        sales[salesCount].discountPercent = discountPercent;

        //RECORD PURCHASE TIME
        sales[salesCount].purchaseTime = time(NULL); // current time

        //copy the name into a struct
        strcpy(sales[salesCount].customerName, nameBuffer);
        salesCount++;
    }else {
        printf("Warning: Sales record is full, cannot record this sale. Computer Says No!\n");
    }
    //update global counters
    carStock[carIndex] -= quantity;
    CarsSold += quantity;

    printf(" Thank You \n %s", nameBuffer);
    printf("\n You have successfully purchased %hu x %s \n", quantity, carModels[carIndex]);
    printf("\n For a total of $%.2f\n", finalPrice);
    printf("\n There are now %d of this model left in stock.\n", carStock[carIndex]);
    askForFeedback(carIndex);
}
void menu_viewSales() {
    printf(" You Have Chosen View Sales Data:\n");
    printf(" Press Enter to Continue...\n");
    getchar();
    if (salesCount == 0) {
        printf("\n No Sales Have Been Made Yet.\n");
        return;
    }
    //arrays to bring together totals per model
    float totalSalesPerModel[MAX_CARS] = {0.0f}; //total revenue per model
    int unitsSoldPerModel[MAX_CARS] = {0}; //units sold per model

    float grandTotal = 0.0f; // revenue across all models
    int grandUnits = 0; // units across all models

    //Loop Through each sale and print basic info to build totals
    for (int i = 0; i <salesCount; i++) {
        int idx = sales[i].carIndex; //which model the sale was for

        char dateStr[26];
        struct tm *tm_info = localtime(&sales[i].purchaseTime);
        if (tm_info != NULL) {
            strftime(dateStr, sizeof(dateStr), " %Y - %m - %d %H:%M:%S", tm_info);
        } else {
            strcpy(dateStr, " Unknown Date/Time");
        }

        printf(" Sale %d\n", i + 1);
        printf(" Customer Name: %s\n", sales[i].customerName);
        printf(" Car Model: %s (%d)\n", carModels[idx], years[idx]);
        printf(" Quantity: %hu\n", sales[i].quantity);
        printf(" Total Price: $%.2f\n", sales[i].totalPrice);

        if (sales[i].discountApplied) {
            printf(" Discount Applied: Yes (%.2f%%)\n", sales[i].discountPercent * 100.0f);
        } else {
            printf(" Discount Applied: No\n");
        }
        printf(" Purchase Date and Time: %s\n", dateStr);
        printf("-------------------------\n");

        totalSalesPerModel[idx] += sales[i].totalPrice;
        unitsSoldPerModel[idx] += sales[i].quantity;

        grandTotal += sales[i].totalPrice;
        grandUnits += sales[i].quantity;
    }
    printf("\n Overall: %d cars sold, total revenue %.2f GBP\n\n", grandUnits, grandTotal);

    //Index list for models: 0,1,2...n_models-1
    int modelIndex[MAX_CARS]; //keep an array called modelIndex with each number in the array pointing to a car model
    for (int i = 0; i < n_models; i++) {
        modelIndex[i] = i; //each position points to a car model and we loop through it
    }

    /*Sorting out (selection sort style), modelIndex[] so that models
     *with higher totalSalesPerModel[] come first.
     * - for each position i, look at all positions j > i
     * -if we find a model with bigger total sale, we swap it into position i
     * for each position i in modelIndex[] this looks at all the models after it (j>i)
     * and finds the one with the highest total sales and swaps it into position i
     */
    for (int i = 0; i < n_models; i++) {
        int bestPos = i; //position with the highest sales

        //look at all models after i
        for (int j = i + 1; j < n_models; j++) {
            int a = modelIndex[bestPos]; //current best model index
            int b = modelIndex[j]; //model we are checking

            //if model b has more total sales than model a, update bestPos
            if (totalSalesPerModel[a] < totalSalesPerModel[b]) {
                bestPos = j;
            }
        }
        //after inner loop, bestPos holds the position of the best model
        //swap it into position i
        int temp = modelIndex[i];
        modelIndex[i] = modelIndex[bestPos];
        modelIndex[bestPos] = temp;
    }
    printf("Sales Summary by Car Model (sorted by total revenue):\n");
    for (int k = 0; k < n_models; k++){
        int idx = modelIndex[k];
        if (unitsSoldPerModel[idx] > 0) {
            printf("%s (%d): %d cars sold, total revenue %.2f GBP\n",
                   carModels[idx],
                   years[idx],
                   unitsSoldPerModel[idx],
                   totalSalesPerModel[idx]);
             }
         }
}
void menu_ViewSalesTargets() {
    printf("You Have Chosen View Sales Targets:\n");
    printf("Press Enter to Continue...\n");
    getchar();

    const int salesTarget = 10; // Example sales target
    printf(" Current Sales Target: %d cars\n", salesTarget);
    printf(" Cars Sold So Far: %d Cars\n", CarsSold);

    if (CarsSold >= salesTarget) {
        printf(" Congratulations! Sales target met or exceeded!\n");
    } else {
        int carsRemaining = salesTarget - CarsSold;
        printf(" Cars remaining to meet target: %d Cars\n", carsRemaining);
    }
}
void menu_feedback() {
    printf(" You have Chosen: View Customer Feedback:\n");
    printf(" Press Enter to Continue...\n");
    getchar();
    if (feedbackCount == 0) {
        printf(" No Feedback Has Been Recorded Yet.\n");
        return;
    }
    //loop through each car model
    for (int m = 0; m <n_models; m++) {
        int hasFeedback = 0; //flag to check if any feedback for this model
        //m (model_index) represents each self in library (car)
        //printf model header only if feedback exists for this model
        for (int i = 0; i < feedbackCount; i++) {
            if (feedbacks[i].carIndex == m) {
                if (!hasFeedback) {
                    printf("\n ===== %s (%d) =====\n", carModels[m], years[m]);
                }
                printf(" \n STAR Rating: %d/5 --- Comment : %s \n",
                    feedbacks[i].rating,
                    feedbacks[i].comment);
                hasFeedback = 1;
            }
        }
        //if not feedback
        if (!hasFeedback) {
            printf("\n%s (%d): No feedback recorded yet.\n", carModels[m], years[m]);
        }
    }
}
void menu_exit() {
    printf("Exit Function Called. Goodbye and have a good week!\n");
    printf("Thank you for visiting Jasper's Car Sales. See you soon!\n");
    printf("Exiting...\n");
}

void saveSalesToFile() {
    openFile(SALES_FILE, "w");
    if (fileStatus == FILE_OPENED) {
        //one  line per sale separated by ';' because semicolons make the saved file easier to read and parse safely
        //semicolon is a separator for fields and comma is for decimal point numbers
        /*storing each sale as one line in a text file
         * fprintf writes each field in a known order, so later it can be read back with fscanf
         * in the same format
         */
        for (int i = 0; i < salesCount; i++) {
            fprintf(file, "%s;%d;%d;%hu;%.2f;%d;%.2f;%ld\n",
                    sales[i].customerName, //%s - name
                    sales[i].customerAge, //%d - age
                    sales[i].carIndex, //%d  -which car model
                    sales[i].quantity, //%hu - how many cars
                    sales[i].totalPrice, //%.2f - final price
                    sales[i].discountApplied, //%d - 0 or 1
                    sales[i].discountPercent, //%.2f - discount proportion (e.g. 0.25)
                    (long)sales[i].purchaseTime //%ld - time as long integer
                    );
        }

        printf("Yay! Sales data successfully saved.\n ");
    } else if (fileStatus == FILE_ERROR) {
        printf("There was an error trying to write the file %s.\n", SALES_FILE);
        pauseProgram('_'); // needs a char so this gives it one in function.
    }

    closeFile(); //closes the global file
}

void loadSalesFromFile() {
    openFile(SALES_FILE, "r");
    if (fileStatus != FILE_OPENED) {
        printf("No existing sales data found. Starting fresh.\n");
        return;
    }
    salesCount = 0;

    while (salesCount < MAX_SALES) {
        Sale *s = &sales[salesCount];
/*the %49[^;] means read up to 49 characters
 * using result to check how many items where read
 */
        int result = fscanf(file, " %49[^;]; %d; %d; %hu; %f; %d; %f; %ld",
                s->customerName,
                &s->customerAge,
                &s->carIndex,        //using & as these are numbers stored in variables and need &
                &s->quantity,
                &s->totalPrice,   //& means store the result in this mem location
                &s->discountApplied,
                &s->discountPercent,
                (long *)&s->purchaseTime
        );

        if (result == EOF) {
            break; //end of file (EOF - no more lines)
        }
        /*this is included as 8 things were asked for above
        *prevents and guards the program from crashing if there was a typo
        *or if there are incorrect values in sales data
        *this acts like s safety shield as bad data can crash the program*/
        if (result != 8) {
            //bad line format - stop loading to avoid corrupt data
            printf(" Warning: Invalid line is sales file, stopping load.\n");
            break; //invalid line
        }
        salesCount++;
    }

    closeFile();

    printf(" Loaded %d sales from file.\n", salesCount);

    CarsSold = 0;
    for (int i = 0; i < salesCount; i++) {
        CarsSold += sales [i].quantity;
    }
}

/* Starting Point of the Program */
int main(void) {
    char userChoice;

    loadSalesFromFile(); //load previous data once at startup

    do {
        clearScreen();
        menu_greeting();
        menu_options();

        userChoice = getMenuchoice(" Please Enter Your Choice Thanks: ");

        clearScreen();

        switch (userChoice) {
            case MENU_BUY_CAR:
                menu_buyCar();
                break;
            case MENU_VIEW_SALES:
                menu_viewSales();
                break;
            case MENU_VIEW_SALES_TARGET:
                menu_ViewSalesTargets();
                break;
            case MENU_VIEW_AVAILABLE_CARS:
                displayAvailableCars();
                break;
            case MENU_FEEDBACK:
                menu_feedback();
                break;
            case MENU_EXIT:
                saveSalesToFile();
                menu_exit();
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }

        pauseProgram(userChoice); /* allows the user to pause read the information and press enter to continue
        it links to getchar(); with out this the program would just flash through on screen and return to menu
        user may get flustered like a deer in  headlights*/

    } while (userChoice != MENU_EXIT);

    clearScreen();
    printf("Have a great day!\n");

    return 0;
}

