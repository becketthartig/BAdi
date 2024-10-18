% data = readtable("2023-09-11.csv"); %Use when need new data
data_aapl = data(data.ticker == "AAPL", :);
avg_aapl = sum(data_aapl.price)/length(data_aapl.price);
counter = 0;
for i = 1:1:722444
    if contains(data_aapl.conditions(i), "10") == 1
        counter = counter + 1;
    end
end

plot(data_aapl.price);





