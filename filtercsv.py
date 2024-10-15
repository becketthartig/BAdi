import pandas as pd

FILEPATH = 'AAPL_ONLY_CSV_WK091123/filteredaapld5.csv'

df = pd.read_csv(FILEPATH) 

#### numbers to filter: 7, 9, 10, 12, 22, 32, 35, 41, 53     (15, 16)?
#### also filter if: correction != 0

numbers_to_filter = {'7', '9', '10', '12', '20', '22', '32', '35', '41', '53'}   # Maybe add 15, 16 

def contains(conditions):
    condition_list = map(str.strip, str(conditions).split(','))
    return any(cond in numbers_to_filter for cond in condition_list)

filtered_rows = df[~df['conditions'].apply(contains) & (df['correction'] == 0)]

filtered_rows.to_csv(FILEPATH[:-4] + 'clean.csv', index=False)