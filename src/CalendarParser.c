#include "CalendarParser.h"
#include "LinkedListAPI.h"


ICalErrorCode SafeErrorHandle(Calendar * ical, FILE * file, Calendar ** obj, ICalErrorCode error) {
    deleteCalendar(ical);
    if(file) {
        fclose(file);
    }
    *obj = NULL;
    return error;
}

bool ValidFile(char * fileName){
    char line[256];
    char *token;
    char tempFile[256];
    FILE *file;

    if (fileName == NULL || strcmp(fileName, "") == 0){
        return false;
    }
    strcpy(tempFile, fileName);

    token = strtok(tempFile, ".");
    if(token != NULL) {
        token = strtok(NULL, ".");
    }

    if(!token || strcmp(token, "ics") != 0) {
        return false;
    }

    file = fopen(fileName, "r");
    if(!file) {
        return false;
    }

    if(!fgets(line, sizeof(line), file)) {
        fclose(file);
        return false;
    }

    while (fgets(line, sizeof(line), file)){
        if(line[strlen(line) - 2] != '\r' && line[strlen(line) - 1] != '\n') {
            fclose(file);
            return false;
        }
    }
    fclose(file);
    return true;
}

bool UnFold(char line[256], char folded[256], FILE * file, char * foldChar){
    char temp[256];
    if(*foldChar != '0') {
        strcpy(temp, line);
        strcpy(line, "");

        line[0] = *foldChar;
        line[1] = '\0';
        strcat(line, temp);

        *foldChar = '0';
    }
    if(line[0] == ';') {
        return false;
    }
    line[strcspn(line, "\r\n")] = 0;
    while(isspace(*foldChar = fgetc(file))) {
        fgets(folded, 257, file);
        folded[strcspn(folded, "\r\n")] = 0;
        strcat(line, folded);
    }
    return true;
}

char * GetInfo(char line[256], char * tag, char * data){
    char *token;
    token = strtok(line, ":;");
    strcpy(token, tag);
    token = strtok(NULL, "");

    return token;
}
char * GetData(char line[256]){
    char *token;
    token = strtok(line, ":;");
    token = strtok(NULL, "");
    return token;
}

ReturnData CreateProperty(char tag[256], char data[256]){
    ReturnData rtn;
    Property * temp = malloc(sizeof(Property) + sizeof(char) * strlen(data) + 1);

    rtn.data = temp;
    if (temp == NULL){
        rtn.error = OTHER_ERROR;
    }

    strcpy(temp->propName, tag);
    strcpy(temp->propDescr, data);
    rtn.error = OK;
    return rtn;
}

ReturnData CreateAlarm(FILE * file, char * c){
    char *token;
    char line[256];
    char * foldChar = c;
    char foldedLine[256];
    char tag[256];
    char data[256];
    bool triggerFlag = false;
    bool actionFlag = false;
    ReturnData rtn;
    

    Alarm * rtnAlarm = malloc(sizeof(Alarm));

    rtnAlarm->properties = initializeList(printProperty, deleteProperty, compareProperties);

    rtn.data = rtnAlarm;
    rtn.error = INV_ALARM;

    if (rtnAlarm == NULL){
        rtn.error = OTHER_ERROR;
        return rtn;
    }

    while(fgets(line, sizeof(line), file)){
        if(!UnFold(line, foldedLine, file, foldChar)){
            continue;
        }

        token = strtok(line, ":;");
        if (token == NULL){
            return rtn;
        }
        strcpy(tag, token);
        token = strtok(NULL, "");
        if (token == NULL){
            return rtn;
        }
        strcpy(data, token);

        if (tag == NULL || strlen(tag) == 0){
            return rtn;
        }

        if (strcmp(tag, "ACTION") == 0){
            if (data == NULL || strlen(data) == 0){
                return rtn;
            }
            else{
                strcpy(rtnAlarm->action, data);
                actionFlag = true;
            }
        }
        else if (strcmp(tag, "TRIGGER") == 0){
            if (data == NULL || strlen(data) == 0){
                rtn.error = INV_DT;
                return rtn;
            }
            else{
                rtnAlarm->trigger = malloc(sizeof(char) * strlen(data) + 1);
                if(rtnAlarm->trigger == NULL){
                    rtn.error = OTHER_ERROR;
                    return rtn;
                }
                strcpy(rtnAlarm->trigger, data);
                triggerFlag = true;
            }
        }
        else if (strcmp(tag, "END") == 0 && strcmp(data, "VALARM") == 0){
            if (actionFlag && triggerFlag){
                rtn.error = OK;
                return rtn;
            }
            else{
                return rtn;
            }
        }
        else if (strcmp(tag, "BEGIN") == 0){
            return rtn;
        }
        else{
            ReturnData prop = CreateProperty(tag, data);
            if (prop.error == OK){
                insertFront(rtnAlarm->properties, prop.data);
            }
            else{
                rtn.error = prop.error;
                return rtn;
            }
        }
    }
    return rtn;
}

ReturnData CreateEvent(FILE * file, char * c){
    char *token;
    char line[256];
    char * foldChar = c;
    char foldedLine[256];
    char tag[256];
    char data[256];
    bool UIDFlag = false;
    bool creationFlag = false;
    bool startFlag = false;
    ReturnData rtn;
    

    Event * rtnEvent = malloc(sizeof(Event));


    strcpy(rtnEvent->UID, "");

    rtnEvent->properties = initializeList(printProperty, deleteProperty, compareProperties);
    rtnEvent->alarms = initializeList(printAlarm, deleteAlarm, compareAlarms);

    rtn.data = rtnEvent;
    rtn.error = INV_EVENT;

    if (rtnEvent == NULL){
        rtn.error = OTHER_ERROR;
        return rtn;
    }

    while(fgets(line, sizeof(line), file)){
        if(!UnFold(line, foldedLine, file, foldChar)){
            continue;
        }

        token = strtok(line, ":;");
        if (token == NULL){
            return rtn;
        }
        strcpy(tag, token);
        token = strtok(NULL, "");
        if (token == NULL){
            return rtn;
        }
        strcpy(data, token);

        if (tag == NULL || strlen(tag) == 0){
            return rtn;
        }

        if (strcmp(tag, "UID") == 0){
            if (data == NULL || strlen(data) == 0){
                return rtn;
            }
            else{
                strcpy(rtnEvent->UID, data);
                UIDFlag = true;
            }
        }
        else if (strcmp(tag, "DTSTAMP") == 0){
            if (data == NULL || strlen(data) == 0){
                rtn.error = INV_DT;
                return rtn;
            }
            else{
                DateTime dt;

                token = strtok(data, "T");
                if(!token || strlen(token) != 8) {
                    rtn.error = INV_DT;
                    return rtn;
                }

                strcpy(dt.date, token);
                token = strtok(NULL, "T");
                if(!token || (strlen(token) != 7 && strlen(token) != 6)) {
                    rtn.error = INV_DT;
                    return rtn;
                }

                dt.UTC = 0;
                if(token[6] == 'Z') {
                    dt.UTC = 1;
                }

                token[6] = '\0';
                strcpy(dt.time, token);
                rtnEvent->creationDateTime = dt;
                creationFlag = true;
            }
        }
        else if (strcmp(tag, "DTSTART") == 0){
            if (data == NULL || strlen(data) == 0){
                rtn.error = INV_DT;
                return rtn;
            }
            else{
                DateTime dt;

                token = strtok(data, "T");
                if(!token || strlen(token) != 8) {
                    rtn.error = INV_DT;
                    return rtn;
                }

                strcpy(dt.date, token);
                token = strtok(NULL, "T");
                if(!token || (strlen(token) != 7 && strlen(token) != 6)) {
                    rtn.error = INV_DT;
                    return rtn;
                }

                dt.UTC = 0;
                if(token[6] == 'Z') {
                    dt.UTC = 1;
                }

                token[6] = '\0';
                strcpy(dt.time, token);
                rtnEvent->startDateTime = dt;
                startFlag = true;
            }
        }
        else if (strcmp(tag, "END") == 0 && strcmp(data, "VEVENT") == 0){
            if (UIDFlag && creationFlag && startFlag){
                rtn.error = OK;
                return rtn;
            }
            else{
                return rtn;
            }
        }
        else if (strcmp(tag, "BEGIN") == 0){
            if (strcmp(data, "VALARM") == 0){
                ReturnData temp = CreateAlarm(file, foldChar);
                if (temp.error == OK){
                    insertFront(rtnEvent->alarms, temp.data);
                }
                else{
                    deleteAlarm(temp.data);
                    rtn.error = temp.error;
                    return rtn;
                }
            }else{
                return rtn;
            }
            
        }
        else{
            ReturnData prop = CreateProperty(tag, data);
            if (prop.error == OK){
                insertFront(rtnEvent->properties, prop.data);
            }
            else{
                rtn.error = prop.error;
                return rtn;
            }
        }
    }
    return rtn;
}


/**Function to create a Calendar object based on the contents of an iCalendar file.
 *@pre File name cannot be an empty string or NULL.  File name must have the .ics extension.
       File represented by this name must exist and must be readable.
 *@post Either:
        A valid calendar has been created, its address was stored in the variable obj, and OK was returned
        or 
        An error occurred, the calendar was not created, all temporary memory was freed, obj was set to NULL, and the 
        appropriate error code was returned
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param fileName - a string containing the name of the iCalendar file
 *@param a double pointer to a Calendar struct that needs to be allocated
**/
ICalErrorCode createCalendar(char* fileName, Calendar** obj){
    char *token;
    char line[256];
    char foldChar = '0';
    char foldedLine[256];
    char tag[256];
    char data[256];
    bool versionFlag = false;
    bool idFlag = false;

    Calendar *cal = (Calendar*)malloc(sizeof(Calendar));
    if (cal == NULL){
        return SafeErrorHandle(cal, NULL, obj, OTHER_ERROR);
    }
    FILE *file;

    cal->version = 0.0;
    cal->events = initializeList(printEvent, deleteEvent, compareEvents);
    cal->properties = initializeList(printProperty, deleteProperty, compareProperties);


    strcpy(cal->prodID, "");

    if (!ValidFile(fileName)){
        return SafeErrorHandle(cal, NULL, obj, INV_FILE);
    }

    file = fopen(fileName, "r");
    fgets(line, sizeof(line), file);
    while (!UnFold(line, foldedLine, file, &foldChar)){
        fgets(line, sizeof(line), file);
    }

    if(strcmp(line, "BEGIN:VCALENDAR") != 0) {
        return SafeErrorHandle(cal, file, obj, INV_CAL);
    }

    while(fgets(line, sizeof(line), file)){
        if(!UnFold(line, foldedLine, file, &foldChar)){
            continue;
        }

        token = strtok(line, ":;");
        if (token == NULL){
            return SafeErrorHandle(cal, file, obj, INV_CAL);
        }
        strcpy(tag, token);
        token = strtok(NULL, "");
        if (token == NULL){
            return SafeErrorHandle(cal, file, obj, INV_CAL);
        }
        strcpy(data, token);

        if (tag == NULL || strlen(tag) == 0){
            return SafeErrorHandle(cal, file, obj, INV_CAL);
        }

        if (strcmp(tag, "VERSION") == 0){
            if (versionFlag){
                return SafeErrorHandle(cal, file, obj, DUP_VER);
            }
            if (data == NULL || strlen(data) == 0){
                return SafeErrorHandle(cal, file, obj, INV_VER);
            }
            else{
                cal->version = atof(data);
                versionFlag = true;
            }
        }

        else if (strcmp(tag, "PRODID") == 0){

            if (idFlag){
                return SafeErrorHandle(cal, file, obj, DUP_PRODID);
            }
            if (data == NULL || strlen(data) == 0){
                return SafeErrorHandle(cal, file, obj, INV_PRODID);
            }
            else{
                strcpy(cal->prodID, data);
                idFlag = true;
            }
        }

        else if (strcmp(tag, "BEGIN") == 0 && strcmp(data, "VEVENT") == 0){
            ReturnData temp = CreateEvent(file, &foldChar);
            if (temp.error == OK){
                insertFront(cal->events, temp.data);
            }
            else{
                deleteEvent(temp.data);
                return SafeErrorHandle(cal, file, obj, temp.error);
            }
        }
        else if (strcmp(tag, "END") == 0 && strcmp(data, "VCALENDAR") == 0){
            if (!versionFlag){
                return SafeErrorHandle(cal, file, obj, INV_VER);
            }
            else if (!idFlag){
                return SafeErrorHandle(cal, file, obj, INV_PRODID);
            }
            else if (getLength(cal->events) == 0){
                return SafeErrorHandle(cal, file, obj, INV_CAL);
            }
            else{
                fclose(file);
                *obj = cal;
                return OK;
            }
        }
        else{
            ReturnData prop = CreateProperty(tag, data);
            if (prop.error == OK){
                insertFront(cal->properties, prop.data);
            }
            else{
                return SafeErrorHandle(cal, file, obj, prop.error);
            }
        }
    }
    return SafeErrorHandle(cal, file, obj, INV_CAL);
}




/** Function to delete all calendar content and free all the memory.
 *@pre Calendar object exists, is not null, and has not been freed
 *@post Calendar object had been freed
 *@return none
 *@param obj - a pointer to a Calendar struct
**/
void deleteCalendar(Calendar* obj) {
    if(obj == NULL){
        return;
    }
    if (obj->events != NULL){
        freeList(obj->events);
    }
    if(obj->properties != NULL){
        freeList(obj->properties);
    }
    free(obj);
    obj = NULL;
    return;
}


/** Function to create a string representation of a Calendar object.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a string representing the Calndar contents has been created
 *@return a string contaning a humanly readable representation of a Calendar object
 *@param obj - a pointer to a Calendar struct
**/
char* printCalendar(const Calendar* obj){
    char *returnString;
    if (obj == NULL || !obj->version || obj->events == NULL || obj->properties == NULL){
        returnString = malloc(sizeof(char) * 1);
        strcpy(returnString, "");
        return returnString;
    }
    char buffer[20];
    char * tempString;
    int length;

    snprintf(buffer, 20, "%.2f", obj->version);

    returnString = malloc(sizeof(char) * (strlen(buffer) + strlen(obj->prodID) + 36));
    sprintf(returnString, "Version: %s\nProduct ID: %s\nProperties:\n", buffer, obj->prodID);
    length = strlen(returnString);


    ListIterator iterator = createIterator(obj->properties);
    if (iterator.current != NULL){
        while(iterator.current != NULL) {
            tempString = obj->properties->printData(iterator.current->data);
            returnString = realloc(returnString, sizeof(char) * (length + strlen(tempString) + 1));
            strcat(returnString, tempString);
            length = strlen(returnString);

            nextElement(&iterator);
            free(tempString);
        }
    }

    iterator = createIterator(obj->events);
    if (iterator.current != NULL){
        while(iterator.current != NULL) {
            tempString = obj->events->printData(iterator.current->data);
            returnString = realloc(returnString, sizeof(char) * (length + strlen(tempString) + 1));
            strcat(returnString, tempString);
            length = strlen(returnString);

            nextElement(&iterator);
            free(tempString);
        }
    }

    return returnString;

}


/** Function to "convert" the ICalErrorCode into a humanly redabale string.
 *@return a string contaning a humanly readable representation of the error code by indexing into 
          the descr array using rhe error code enum value as an index
 *@param err - an error code
**/
char* printError(ICalErrorCode err){
    char *str;

    switch(err) {
        case OK:
            str = "OK";
            break;
        case INV_FILE:
            str = "Invalid File";
            break;
        case INV_CAL:
            str = "Invalid Calendar";
            break;
        case INV_VER:
            str = "Invalid Version";
            break;
        case DUP_VER:
            str = "Multiple Versions Detected";
            break;
        case INV_PRODID:
            str = "Invalid Product ID";
            break;
        case DUP_PRODID:
            str = "Multiple Product IDs Detected";
            break;
        case INV_EVENT:
            str = "Invalid Event";
            break;
        case INV_DT:
            str = "Invalid DateTime";
            break;
        case INV_ALARM:
            str = "Invalid Alarm";
            break;
        case WRITE_ERROR:
            str = "Write Error";
            break;
        case OTHER_ERROR:
            str = "Unknown Error";
            break;
        default:
            str = "Unknown Error";
            break;
    }
    return str;
}

/** Function to writing a Calendar object into a file in iCalendar format.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a file representing the
        Calendar contents in iCalendar format has been created
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param obj - a pointer to a Calendar struct
 **/
//DONT IMPLEMENT
ICalErrorCode writeCalendar(char* fileName, const Calendar* obj){
    ICalErrorCode temp = OK;
    return temp;
}


/** Function to validating an existing a Calendar object
 *@pre Calendar object exists and is not null
 *@post Calendar has not been modified in any way
 *@return the error code indicating success or the error encountered when validating the calendar
 *@param obj - a pointer to a Calendar struct
 **/
//DONT IMPLEMENT 
ICalErrorCode validateCalendar(const Calendar* obj){
    ICalErrorCode temp = OK;
    return temp;
}

void deleteEvent(void* toBeDeleted){
    if(toBeDeleted == NULL) {
        return;
    }

    Event * tempEvent = (Event*)toBeDeleted;
    freeList(tempEvent->properties);
    freeList(tempEvent->alarms);
    free(toBeDeleted);
}

int compareEvents(const void* first, const void* second){
    Event * eventOne;
    Event * eventTwo;

    if(first == NULL || second == NULL) {
        return -1;
    }

    eventOne = (Event*)first;
    eventTwo = (Event*)second;

    return strcmp(eventOne->UID, eventTwo->UID);
}

char* printEvent(void* toBePrinted){
    char *returnString;
    char * tempString;
    int length;

    if(toBePrinted == NULL) {
        return NULL;
    }

    Event * tempEvent = toBePrinted;
    returnString = malloc(sizeof(char) * strlen(tempEvent->UID) + 16);
    sprintf(returnString, "\tEvent:\n\tUID: %s\n", tempEvent->UID);
    length = strlen(returnString);

    tempString = printDate(&(tempEvent->creationDateTime));
    returnString = realloc(returnString, sizeof(char) * (length + strlen(tempString) + 21));
    strcat(returnString, "\tCreation DateTime: ");
    strcat(returnString, tempString);
    free(tempString);
    length = strlen(returnString);

    tempString = printDate(&(tempEvent->startDateTime));
    returnString = realloc(returnString, sizeof(char) * (length + strlen(tempString) + 18));
    strcat(returnString, "\tStart DateTime: ");
    strcat(returnString, tempString);
    free(tempString);
    length = strlen(returnString);

    returnString = realloc(returnString, sizeof(char) * (length + 14));
    strcat(returnString, "\tProperties:\n");
    length = strlen(returnString);

    ListIterator iterator = createIterator(tempEvent->properties);
    if (iterator.current != NULL){
        while(iterator.current != NULL) {
            tempString = tempEvent->properties->printData(iterator.current->data);
            returnString = realloc(returnString, sizeof(char) * (length + strlen(tempString) + 2));
            strcat(returnString, "\t");
            strcat(returnString, tempString);
            length = strlen(returnString);

            nextElement(&iterator);
            free(tempString);
        }
    }
    

    iterator = createIterator(tempEvent->alarms);
    if (iterator.current != NULL){
        while(iterator.current != NULL) {
            tempString = tempEvent->alarms->printData(iterator.current->data);
            returnString = realloc(returnString, sizeof(char) * (length + strlen(tempString) + 2));
            strcat(returnString, "\t");
            strcat(returnString, tempString);
            length = strlen(returnString);

            nextElement(&iterator);
            free(tempString);
        }
    }

    returnString = realloc(returnString, sizeof(char) * (length + 2));
    strcat(returnString, "\n");

    return returnString;
}

void deleteAlarm(void* toBeDeleted){
    if(toBeDeleted == NULL) {
        return;
    }

    Alarm * tempAlarm = toBeDeleted;
    freeList(tempAlarm->properties);

    if(tempAlarm->trigger) {
        free(tempAlarm->trigger);
    }

    free(toBeDeleted);
}

int compareAlarms(const void* first, const void* second){
    Alarm * alarmOne;
    Alarm * alarmTwo;

    if(first == NULL || second == NULL) {
        return -1;
    }

    alarmOne = (Alarm*)first;
    alarmTwo = (Alarm*)second;

    return strcmp(alarmOne->action, alarmTwo->action);
}

char* printAlarm(void* toBePrinted){
    char *returnString;
    char * tempString;
    int length;

    if(toBePrinted == NULL) {
        return NULL;
    }

    Alarm * tempAlarm = toBePrinted;
    returnString = malloc(sizeof(char) * (strlen(tempAlarm->action) + strlen(tempAlarm->trigger) + 51));
    sprintf(returnString, "\t\tAlarm:\n\t\t\tAction: %s\n\t\t\tTrigger: %s\n\t\t\tProperties:\n", tempAlarm->action, tempAlarm->trigger);
    length = strlen(returnString);

    ListIterator iterator = createIterator(tempAlarm->properties);
    if (iterator.current == NULL){
        returnString = realloc(returnString, sizeof(char) * (length + 2));
        strcat(returnString, "\n");
        return returnString;
    }
    while(iterator.current != NULL) {
        tempString = tempAlarm->properties->printData(iterator.current->data);
        returnString = realloc(returnString, sizeof(char) * (length + strlen(tempString) + 4));
        strcat(returnString, "\t\t\t");
        strcat(returnString, tempString);
        length = strlen(returnString);

        nextElement(&iterator);
        free(tempString);
    }

    returnString = realloc(returnString, sizeof(char) * (length + 2));
    strcat(returnString, "\n");

    return returnString;
}

void deleteProperty(void* toBeDeleted){
    if(toBeDeleted == NULL) {
        return;
    }
    free(toBeDeleted);
    return;
}

int compareProperties(const void* first, const void* second){
    Property * firstProperty;
    Property * secondProperty;

    if (first == NULL || second == NULL) {
        return -1;
    }

    firstProperty = (Property*)first;
    secondProperty = (Property*)second;

    return strcmp(firstProperty->propName, secondProperty->propName);
}

char* printProperty(void* toBePrinted){
    char *returnString;

    if(toBePrinted == NULL) {
        return NULL;
    }

    Property *tempProperty = toBePrinted;
    returnString = malloc((strlen(tempProperty->propName) + strlen(tempProperty->propDescr) + 5) * sizeof(char));
    sprintf(returnString, "- %s:%s\n", tempProperty->propName, tempProperty->propDescr);

    return returnString;
}

void deleteDate(void* toBeDeleted){
    if(toBeDeleted == NULL) {
        return;
    }
    free(toBeDeleted);
    return;
}

int compareDates(const void* first, const void* second){
    DateTime * firstDateTime;
    DateTime * secondDateTime;

    if (first == NULL || second == NULL) {
        return -1;
    }

    firstDateTime = (DateTime*)first;
    secondDateTime = (DateTime*)second;

    int returnInt = 0;
    returnInt += strcmp(firstDateTime->date, secondDateTime->date);
    returnInt += strcmp(firstDateTime->time, secondDateTime->time);
    if (firstDateTime->UTC == secondDateTime->UTC){
        return returnInt;
    }
    return -1;
}

char* printDate(void* toBePrinted){
    char *returnString;

    if(toBePrinted == NULL) {
        return NULL;
    }

    DateTime *tempDateTime = toBePrinted;
    returnString = malloc((strlen(tempDateTime->date) + strlen(tempDateTime->time) + 27) * sizeof(char));
    sprintf(returnString, "Date:%s Time:%s UTC:", tempDateTime->date, tempDateTime->time);
    if (tempDateTime->UTC){
        strcat(returnString, "True\n");
    }
    else{
        strcat(returnString, "False\n");
    }

    return returnString;
}