-- 脚本由kdxcxs 友情提供 https://github.com/kdxcxs

local g = golly()

local gx, gy, wd, ht = table.unpack(g.getrect())
local cells = g.getcells(g.getrect())
local count = wd * ht
local data = {}

for i=1,count do
    data[i] = 0
end

local fname = g.savedialog("Export csv file", "CSV files (*.csv)|*.csv", ".", "exported.csv")
local file = io.open(fname, "w")

local len = #cells
local inc = 2
if len & 1 == 1 then
    -- multi-state cell array so set inc to 3 and ignore any padding int
    inc = 3
    if len % 3 == 1 then len = len - 1 end
end

for i = 1, len, inc do
    local x = cells[i] - gx
    local y = cells[i+1] - gy
    local state = 1
    if inc == 3 then state = cells[i+2] end
    data[y * wd + x + 1] = state
end

file:write("Index,IsTrue\n")

for i = 1, count do
    if data[i] == 1 then
        file:write(i..",".."\"True\"\n")
    else
        file:write(i..",".."\"False\"\n")
    end
end

file:close()

local log = io.open("ExortLog.log", "w")
log:write(os.date(),":  Exported csv file!  ", "Width:", wd, "  Height:", ht, "\n")
log:close()
