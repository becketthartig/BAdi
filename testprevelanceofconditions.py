import csv
import time

line_count = 0
condition_count = 0
init_time = time.time()

with open('2023-09-11.csv', mode ='r') as f:

    data = csv.reader(f)

    for line in data:

        if '12' in line[1]:
            condition_count = condition_count + 1

        line_count = line_count + 1
        if line_count % 1000000 == 0:
            print(f'Trades analyzed: {line_count}')

        
print('----**** Done ****----------------')
print(f'Times Seen Condition: {condition_count}')
print(f'Time to execute (s): {time.time() - init_time}')