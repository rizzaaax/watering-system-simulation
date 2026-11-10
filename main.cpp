#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <clocale>

using namespace std;

// Константы задачи
const int DAYS_PER_YEAR = 365;
const double START_MOISTURE = 50.0;
const double RAIN_AMOUNT = 20.0;
const double EVAP_AMOUNT = 8.0;
const double WATER_AMOUNT = 30.0;
const double LITERS_PER_WATER = 10.0;
const double DRY_THRESHOLD = 25.0;
const int NUM_EXPERIMENTS = 1000;

// Структура для хранения средних результатов
struct Result {
    double avg_liters;
    double avg_waterings;
    double avg_dry_days;
};

// Функция ограничения влажности в диапазоне [0, 100]
double limit_moisture(double val) {
    if (val < 0.0) return 0.0;
    if (val > 100.0) return 100.0;
    return val;
}

// Моделирование 1000 лет для одной пары параметров
Result run_simulation(int threshold, int rain_prob_percent) {
    double total_liters = 0.0;
    double total_waterings = 0.0;
    double total_dry_days = 0.0;

    for (int exp = 0; exp < NUM_EXPERIMENTS; exp++) {
        double moisture = START_MOISTURE;
        int waterings_count = 0;
        int dry_days_count = 0;

        for (int day = 1; day <= DAYS_PER_YEAR; day++) {
            // 1. Проверка дождя по вероятности
            if (rand() % 100 < rain_prob_percent) {
                moisture = limit_moisture(moisture + RAIN_AMOUNT);
            }

            // 2. Испарение
            moisture = limit_moisture(moisture - EVAP_AMOUNT);

            // 3. Датчик с погрешностью от -5 до +5
            int sensor_err = (rand() % 11) - 5; 
            double sensor_val = moisture + sensor_err;

            // 4. Полив, если показания ниже порога
            if (sensor_val < threshold) {
                moisture = limit_moisture(moisture + WATER_AMOUNT);
                waterings_count++;
            }

            // 5. Проверка на сухой день в конце суток
            if (moisture < DRY_THRESHOLD) {
                dry_days_count++;
            }
        }

        total_waterings += waterings_count;
        total_liters += waterings_count * LITERS_PER_WATER;
        total_dry_days += dry_days_count;
    }

    Result res;
    res.avg_liters = total_liters / NUM_EXPERIMENTS;
    res.avg_waterings = total_waterings / NUM_EXPERIMENTS;
    res.avg_dry_days = total_dry_days / NUM_EXPERIMENTS;
    return res;
}

// Ручная верификация на 10 дней с детерминированным дождем
void run_verification() {
    cout << "\n=============================================================\n";
    cout << " ВЕРИФИКАЦИЯ: ручной прогон на 10 дней (порог 40%, ошибка датчика = 0)\n";
    cout << "=============================================================\n";

    // Дождь идет только в дни 1, 3 и 6
    bool rain_days[10] = {true, false, true, false, false, true, false, false, false, false};
    double moisture = START_MOISTURE;
    int waterings = 0;

    cout << fixed << setprecision(1);

    for (int day = 0; day < 10; day++) {
        double start_val = moisture;

        if (rain_days[day]) {
            moisture = limit_moisture(moisture + RAIN_AMOUNT);
        }
        moisture = limit_moisture(moisture - EVAP_AMOUNT);

        // Погрешность датчика по условию верификации = 0
        double sensor_val = moisture;
        bool watered = false;

        if (sensor_val < 40.0) {
            moisture = limit_moisture(moisture + WATER_AMOUNT);
            waterings++;
            watered = true;
        }

        cout << "День " << setw(2) << day + 1 
             << ": старт=" << setw(4) << start_val
             << " | дождь=" << (rain_days[day] ? "Да " : "Нет")
             << " | датчик=" << setw(4) << sensor_val
             << " | " << (watered ? "ПОЛИВ " : "      ")
             << " | итог=" << setw(4) << moisture << "\n";
    }

    cout << "Всего поливов за 10 дней: " << waterings 
         << " (" << waterings * LITERS_PER_WATER << " л)\n";
}

int main() {
    // Инициализация генератора случайных чисел
    srand(42); 
    setlocale(LC_ALL, "Russian");


    int thresholds[3] = {30, 40, 50};
    int rain_probs[3] = {0, 20, 40};
    Result table[3][3];

    cout << "Выполняются расчеты...\n";

    // Проводим эксперименты для всех 9 сочетаний
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            table[i][j] = run_simulation(thresholds[j], rain_probs[i]);
        }
    }

    // Вывод таблицы
    cout << "\n-------------------- СВОДНАЯ ТАБЛИЦА --------------------\n";
    cout << fixed << setprecision(1);
    cout << "Дождь \\ Порог\t| Порог 30%\t| Порог 40%\t| Порог 50%\n";
    cout << "---------------------------------------------------------\n";

    for (int i = 0; i < 3; i++) {
        // Расход воды
        cout << "Дождь " << rain_probs[i] << "% (л)\t| ";
        for (int j = 0; j < 3; j++) {
            cout << table[i][j].avg_liters << " л\t| ";
        }
        cout << "\n";

        // Число поливов
        cout << "  (поливы)\t| ";
        for (int j = 0; j < 3; j++) {
            cout << table[i][j].avg_waterings << " раз\t| ";
        }
        cout << "\n";

        // Сухие дни
        cout << "  (сухие дни)\t| ";
        for (int j = 0; j < 3; j++) {
            cout << table[i][j].avg_dry_days << " дн\t| ";
        }
        cout << "\n---------------------------------------------------------\n";
    }

    // Поиск лучшего варианта
    double min_liters = 1e9;
    int best_rain = -1;
    int best_thresh = -1;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (table[i][j].avg_dry_days < 10.0 && table[i][j].avg_liters < min_liters) {
                min_liters = table[i][j].avg_liters;
                best_rain = rain_probs[i];
                best_thresh = thresholds[j];
            }
        }
    }

    cout << "\n[Оптимальные параметры]\n";
    if (best_rain != -1) {
        cout << "Лучший вариант: Дождь = " << best_rain 
             << "%, Порог = " << best_thresh 
             << "%, расход = " << min_liters << " л.\n";
    } else {
        cout << "Вариант с числом сухих дней < 10 не найден.\n";
    }

    // Запуск контрольного теста
    run_verification();

    return 0;
}