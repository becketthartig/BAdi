// cleandata.cpp
// Author: Beckett Hartig
// Company: Harth in Stock*
//
// cleandata.cpp is a file designed to clean the data within
// a csv file of a day of trading data downloaded from Polygon.io
// based on a host of criteria
//
// Criteria for filtering out a line of data include:
// - Trade condition marked as 7, 9, 10, 12, 20, 22, 32, 35, 41, or 53
// - Trade is marked with a NYSE trade corrections
// - Trade has a volume of one and occurs within the first 10 minutes of the trading day
//
// how to run cleandata:
// - within terminal or cmd (in VS Code), navigate to the directory where the file is located
// - enter the command: g++ -o cleandata cleandata.cpp -O2 -std=c++17
// - enter command to run: ./cleandata
// - Notes:
//   - within the main function you must set the correct date
//   - within the main function you must propely set file paths for the in and out files

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <cmath> 
#include <deque>
#include <chrono>
#include <ctime>

// no addtl include files for simplicity

// no namespace

// convert_to_unix_timestamp
// takes a date and time, parses string, returns UNIX nanosecond time stamp
// accoring to GMT not local time or EST
long long convert_to_unix_timestamp(const std::string& date, const std::string& time) {
    // Convert input date and time to a tm structure
    std::tm tm = {};
    std::istringstream ss(date + " " + time);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    // Convert tm to time_t (seconds since epoch GMT)
    std::time_t time_since_epoch = timegm(&tm);

    // Convert to nanoseconds
    auto duration = std::chrono::seconds(time_since_epoch);
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

    return nanoseconds;
}

// should_filter
// function to check if any condition matches the bad conditions
// pass parsed conditions string and set of bad conditions
bool should_filter(const std::string& conditions, const std::set<std::string>& numbers_to_filter) {
    if (conditions.empty()) {
        return false;  // No conditions to filter, so return false
    }

    std::stringstream ss(conditions);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        // Check if the condition is in the filter set
        if (numbers_to_filter.find(item) != numbers_to_filter.end()) {
            return true;
        }
    }
    
    return false;  // No conditions matched the filter
}

// parse_csv_row
// Custom function to properly parse CSV rows, handling quoted fields
// pass string line from csv
std::vector<std::string> parse_csv_row(const std::string& line) {
    std::vector<std::string> result;
    bool inside_quotes = false;
    std::string temp;

    for (char ch : line) {
        if (ch == '"') {
            inside_quotes = !inside_quotes;  // Toggle the inside_quotes flag
        } else if (ch == ',' && !inside_quotes) {
            result.push_back(temp);
            temp.clear();  // Clear the token buffer for the next field
        } else {
            temp += ch;
        }
    }
    result.push_back(temp);  // Add the last field

    return result;
}

// main
// main loop over every line in csv
// edit indicated varible fields in main to achieve desired filter
int main() {

    auto inittime = std::chrono::high_resolution_clock::now(); // catch run time

    const std::string input_file = "CSV_WK091123/2023-09-11.csv"; // path to infile
    const std::string output_file = "911cleaned3.csv"; // path to outfile
    
    // Set of numbers to filter
    std::set<std::string> numbers_to_filter = {"7", "9", "10", "12", "20", "22", "32", "35", "41", "53"};
    
    std::string date = "2023-09-11"; // Ensure correct date is set (YYYY-MM-DD)
    const std::string time = "13:30:00"; // DO NOT CHANGE (GMT for beinging of trading day in US)
    long long nstimestamp = convert_to_unix_timestamp(date, time);  // Unix timestamp for begining of trading day
    const long long ten_minutes_ns = 600000000000;  // 10 minutes in nanoseconds

    std::ifstream infile(input_file);
    std::ofstream outfile(output_file);

    // if fail to open csv
    if (!infile.is_open() || !outfile.is_open()) {
        std::cerr << "Error opening file(s)." << std::endl;
        return 1;
    }

    // add header line
    std::string line;
    std::getline(infile, line);
    outfile << line << std::endl;
    int count = 1;

    const int window_size = 20; // STD calculation window size (changeable)
    const double threshold = 0.1 * window_size;
    const double window_size_inv = 1.0 / window_size;

    std::deque<double> moving_prices;
    double moving_sum = 0.0;
    double moving_sq_sum = 0.0;

    std::vector<std::string> header = parse_csv_row(line);
    std::string current_ticker = header[0];
    int last_ticker_i = 0;
    
    // CSV Parsing Loop
    // Parse until end of CSV
    while (std::getline(infile, line)) {

        std::vector<std::string> columns = parse_csv_row(line);

        std::string conditions = columns[1];
        std::string correction_str = columns[2];
        std::string timestamp_str = columns[5];
        std::string size_str = columns[9];

        // Parse correction, timestamp, and size
        int correction = 0;
        long long participant_timestamp = 0;
        int size = 0;
        try {
            correction = std::stoi(correction_str);  
            participant_timestamp = std::stoll(timestamp_str);  
            size = std::stoi(size_str);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing numeric values: " << e.what() << " in line: " << line << std::endl;
            continue;  // Skip row if parsing error
        }

        // if provides initial filter based on non calculated row metadata
        if (!should_filter(conditions, numbers_to_filter) && correction == 0 && 
            !((participant_timestamp - nstimestamp < ten_minutes_ns) && (size == 1))) {

            // if passed initial filters continue to calculate standard deviation
            // standard deviation used as last line of defense from bad data
            // calculations done with Welford's Method for efficiency
            std::string ticker = columns[0];
            if (ticker != current_ticker) { // reset when new stock encountered
                moving_prices.clear();
                moving_sum = 0.0;
                moving_sq_sum = 0.0;
                current_ticker = ticker;
                last_ticker_i = count - 1;
            }

            // adjust sums and deque
            double price = std::stod(columns[6]);
            moving_sum += price;
            moving_sq_sum += price * price;
            moving_prices.push_back(price);

            // only calculate if lines encountered exceed window size
            if (count - last_ticker_i > window_size) {  

                if (moving_prices.size() > window_size) { // only pop front if last line encountered was good
                    double oldest_price = moving_prices.front();
                    moving_prices.pop_front();
                    moving_sum -= oldest_price;
                    moving_sq_sum -= oldest_price * oldest_price;
                }

                double mean = moving_sum * window_size_inv;
                double sum_sq_deviations = moving_sq_sum - 2 * mean * moving_sum + window_size * mean * mean;
       
                // if standard deviation exceeds threshold, remove line and data point
                // remove from further std calculations
                if (sum_sq_deviations > threshold) {
                    moving_sum -= price;
                    moving_sq_sum -= price * price;
                    moving_prices.pop_back();
                    last_ticker_i++;
                } else 
                    outfile << line << std::endl; // write line to new csv

            } else 
                outfile << line << std::endl; // write line to new csv

        } else { // if line excluded increase "index" of the end of the stream from last ticker
            last_ticker_i++;
        }

        if (count % 1000000 == 0) // log every million lines
            std::cout << count << std::endl;

        count++; // always counter increment
    }

    infile.close();
    outfile.close();

    // End print for run time
    std::cout << "----**** Done ****------------" << std::endl;
    auto ttime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - inittime);
    std::cout << "Time to execute (s): " << ttime.count() << std::endl;
    
    return 0;
}
