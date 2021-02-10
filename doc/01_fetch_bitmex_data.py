#!/usr/bin/env python3
import os
import urllib.request
from datetime import timedelta, date

BASE_URL = "https://s3-eu-west-1.amazonaws.com/public.bitmex.com/"

trade_dir = 'data/trade/'

if not os.path.exists(trade_dir):
    os.makedirs(trade_dir)

quote_dir = 'data/quote/'

if not os.path.exists(quote_dir):
    os.makedirs(quote_dir)

# in case the script was stopped, we could skip past already downloaded
offset = 0

days_back = 365 - offset

# sometimes bitmex data is posted a day later, so lets buffer
end_date = date.today() - timedelta(days=2)

start_date = end_date - timedelta(days=days_back)

days_diff = (end_date - start_date).days

print(f'fetching {days_diff} days from {start_date.strftime("%Y%m%d")} to {end_date.strftime("%Y%m%d")}')


for x in range(days_diff):
    print(f'{x+1}  of  {days_diff}')
    target_date = start_date + timedelta(days=x)
    filename = target_date.strftime("%Y%m%d") + ".csv.gz"
    target_trade_url = BASE_URL + trade_dir + filename
    print('Downloading Trades: ', target_trade_url)
    urllib.request.urlretrieve(target_trade_url, trade_dir + filename)
    target_quote_url = BASE_URL + quote_dir + filename
    print('Downloading Quotes: ', target_quote_url)
    urllib.request.urlretrieve(target_quote_url, quote_dir + filename)


