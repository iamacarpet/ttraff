extern pthread_mutex_t log_lock;

void openLog();
void closeLog();
void logWrite(std::string logData);
std::string trim (std::string str);
unsigned int daysformonth(unsigned int month, unsigned int year);
int weekday(int month, int day, int year);