#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <ctime>
#include <iomanip>
#include <vector>

using namespace std;

class TickerClass {
public:
    std::tm Date;
    std::string Ticker;
    double Close;
    double High;
    double Low;
    double Open;
    double Volume;
    double Dividends;
    double Splits;

    void publicMethod() {
        Init();
    }
    void Init() {
        Ticker = "";
        Close = 0;
        High = 0;
        Low = 0;
        Open = 0;
        Volume = 0;
        Dividends = 0;
        Splits = 0;
    }
};

std::string doubleToString(double value, int precision = 2) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

bool stringToDate(const std::string& dateString, std::tm& date) {
    std::istringstream iss(dateString.substr(0,10));
    iss >> std::get_time(&date, "%Y-%m-%d"); // Adjust the format as needed

    return !iss.fail();
}

std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::stringstream ss(str);

        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }

        return tokens;
    }

enum STOCK {
    Ticker,
    Date, 
    Close,  
    High,
    Low,
    Open,
    Volume,
    Dividends,
    Splits
};

bool toDouble(const std::string& str, double &result) {
    std::istringstream iss(str);
    iss >> result;
    return !(iss.fail() || !iss.eof());
}

void getStatsClose(TickerClass a_tc[], int size, double& max_close, double& min_close, double& beg_price, double& end_price) {
    max_close = -1;
    min_close = -1;
    beg_price = -1;
    end_price = a_tc[size-1].Close;

    for (int i = 1; i < size; ++i) {
        if (a_tc[i].Close > max_close){
            max_close = a_tc[i].Close;
        }
        if (a_tc[i].Close > 0 && beg_price < 0) {
            beg_price = a_tc[i].Close;
        }
        if ((a_tc[i].Close > 0 && min_close < 0 ) || a_tc[i].Close < min_close) {
            min_close = a_tc[i].Close;
        }
    }
}

void getStatsLow(TickerClass a_tc[], int size, double& max_low, double& min_low) {
    max_low = -1;
    min_low = -1;

    for (int i = 1; i < size; ++i) {
        if (a_tc[i].Low > max_low) {
            max_low = a_tc[i].Low;
        }
        if ((a_tc[i].Low > 0 && min_low < 0 ) || a_tc[i].Low < min_low) {
            min_low = a_tc[i].Low;
        }
    }
}

void getStatsHigh(TickerClass a_tc[], int size, double& max_high, double& min_high) {
    max_high = a_tc[0].High;
    min_high = a_tc[0].High;

    for (int i = 1; i < size; ++i) {
        if (a_tc[i].High > max_high) {
            max_high = a_tc[i].High;
        }
        if (a_tc[i].High < min_high) {
            min_high = a_tc[i].High;
        }
    }
}

void printList(TickerClass a_tc[], int size){
    cout << std::right
         << "|" << std::setw(10) << "Ticker"
         << "|" << std::setw(12) << "Date"
         << "|" << std::setw(12) << "Close"
         << "|" << std::setw(12) << "High"
         << "|" << std::setw(12) << "Low"
         << "|" << std::setw(12) << "Open"
         << "|" << std::setw(12) << "Volume"
         << "|" << std::setw(12) << "Dividends"
         << "|" << std::setw(12) << "Splits" << endl;

    for (int i = 0; i < size; i++) {
        std::ostringstream oss;
        oss << std::put_time(&a_tc[i].Date, "%Y-%m-%d");
        cout << std::right
             << "|" << std::setw(10) << a_tc[i].Ticker
             << "|" << std::setw(12) << oss.str()
             << "|" << std::setw(12) << a_tc[i].Close
             << "|" << std::setw(12) << a_tc[i].High
             << "|" << std::setw(12) << a_tc[i].Low
             << "|" << std::setw(12) << a_tc[i].Open
             << "|" << std::setw(12) << doubleToString(a_tc[i].Volume,0)
             << "|" << std::setw(12) << a_tc[i].Dividends
             << "|" << std::setw(12) << a_tc[i].Splits << endl;
    } 
    double min_close;
    double max_close;
    double max_high;
    double min_low;
    double ignore;
    double beg_price;
    double end_price;

    end_price = a_tc[size-1].Close;
    getStatsClose(a_tc, size, max_close, min_close, beg_price, end_price);

    cout << "=============One Month Summary==============" << endl;

    cout << "    Max Close = " << max_close << endl;
    cout << "    Min Close = " << min_close << endl;
    
    getStatsLow(a_tc, size, ignore, min_low);
    getStatsHigh(a_tc, size, max_high, ignore);

    cout << " Day's Lowest = " << min_low << endl;
    cout << "Day's Highest = " << max_high << endl;
    cout << " Begin  Price = " << beg_price << endl;
    cout << " Latest Price = " << end_price << endl;

    cout << endl;

    double delta;
    delta = end_price - beg_price;

    cout << ( delta > 1 ? "It is a buy/hold." : "It is a hold or sell." ) << " Projected Profit: $" << delta << endl << endl;

    cout << "=============================================" << endl;

}

int main()
{
    int size = 10000;
    
    char* buffer = new char[size];
    std::string s_val;
    std::ostringstream ss;
    char delimiter = '|';
    TickerClass a_tc[200];
    std::string ticker = "IBM";

    std::cout << "Enter ticker(like IBM or META):  ";
    std::cin >> ticker;

    std::ostringstream oss;
    oss << "C:/Users/jason/AppData/Local/Programs/Python/Python313/python.exe -c \"import yfinance as yf;ticker = yf.Ticker('" << ticker << "');data = ticker.history(period='1mo');print(data.to_csv('C:/Users/jason/temp/out.tab',sep='|'))\"";

    system(oss.str().c_str());

    TickerClass tc = TickerClass();
    std::ifstream file("C:/Users/jason/temp/out.tab");

    int ln_cnt = 0;
    if (file) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.substr(0, 4) == "Date") {
                continue;
            }
            std::vector<std::string> result = split(line, delimiter);
			
            tc.Ticker = ticker;
            int cnt = 1;
            for (const auto& token : result) {
                switch (cnt) {
                case STOCK::Date:
                    stringToDate(token, tc.Date);
                    break;
                case STOCK::Close:
                    toDouble(token, tc.Close);
                    break;
                case STOCK::High:
                    toDouble(token, tc.High);
                    break;
                case STOCK::Low:
                    toDouble(token, tc.Low);
                    break;
                case STOCK::Open:
                    toDouble(token, tc.Open);
                    break;
                case STOCK::Volume:
                    toDouble(token, tc.Volume);
                    break;
                case STOCK::Dividends:
                    toDouble(token, tc.Dividends);
                    break;
                case STOCK::Splits:
                    toDouble(token, tc.Splits);
                    break;
                default:
                    std::cout << "Invalid input!" << std::endl;
                    break;
                }
                cnt++;
            }
            a_tc[ln_cnt++] = tc;

        }
		
    }
    else {
        std::cerr << "Unable to open file" << std::endl;
    }
    printList(a_tc, ln_cnt);
    
}


//This code projects stocks of various companys using their ticker and by analyzing the data you can check which company to invest to
//Examples of companys tickers are IBM, META, AMZN, GOOGL, TSLA, etc.
// For this to run, PC needs to have python installed and package yfinance
// need to be installed with the command pip install yfinance
// I tried to use cpprestapi C++ lbrary and is not working easily on windows
// This process accpet a valid ticket symbol from the input and extract 
//1 month price data from yahoo finance and store it into a class array
//check the max and min close price to see it is a buy or a sell based on profit
//use the command window (cmd) to check the output, and open the exe ".\JsonProjectStock.exe" and enter the ticker