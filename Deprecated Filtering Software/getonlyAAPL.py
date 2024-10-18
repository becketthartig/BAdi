import pandas as pd

df = pd.read_csv('2023-09-11.csv') 

filtered_rows = df[df['ticker'] == 'AAPL'] 

filtered_rows.to_csv('filteredaapl.csv', index=False)
