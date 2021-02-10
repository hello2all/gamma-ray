#!/usr/bin/env python3

# Description:
# After downloading the raw data from BitMex, this script
# separates the data into separate files (one for each symbol) and removes the "symbol" column from the data
# By nature, it performs two sanity checks:
#     1) that the data downloaded is continuous (there isn't a missing file)
#     2) that the csv data can actually be read (by Pandas)

import glob
import re
import datetime
import pandas as pd
from datetime import timedelta, date

def process(file: str, quote_or_trade: str):
    print(quote_or_trade)
    mode = "a"
    header = False
    if x == 0:
        mode = "w"
        header = True
    df = pd.read_csv(f'data/{quote_or_trade}/{file}')

    symbols = df.symbol.unique()
    # df.drop(columns=["symbol"], inplace=True)
    for symbol in symbols:
        # print(f'Extracting {symbol}')
        df[df['symbol'] == symbol].drop(columns=["symbol"]).to_csv(f'data/{symbol}_{quote_or_trade}.csv', mode=mode, header=header)


file_list = sorted(glob.glob("data/*/*.csv.gz"))

first = re.search(f'\d+', file_list[0]).group()  # gives us something like "20210208" to be parsed
last = re.search(f'\d+', file_list[len(file_list) - 1]).group()

start_date = datetime.datetime.strptime(first, '%Y%m%d')
end_date = datetime.datetime.strptime(last, '%Y%m%d')

days_diff = (end_date - start_date).days

print(f'Extracting and concat .gz: {first} to {last}')

for x in range(days_diff + 1):
    print(f'{x + 1}  of  {days_diff + 1}')
    target_date = start_date + timedelta(days=x)
    filename = target_date.strftime('%Y%m%d') + ".csv.gz"
    process(filename, "quote")
    process(filename, "trade")
