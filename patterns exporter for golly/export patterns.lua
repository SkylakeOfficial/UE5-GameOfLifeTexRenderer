-- This script exports game of life patterns from golly

local g = golly()

local gx, gy, wd, ht = table.unpack(g.getrect())
local cells = g.getcells(g.getrect())
local count = wd * ht
local data = {}

for i=1,count do
    data[i] = 0
end

local fname = g.savedialog("Export text file", "Game of life files (*.gol)|*.gol", ".", "exported.gol")
local file = io.open(fname, "w")
file:write(wd..","..ht..",")

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

local function writeRLE(IsTrue, Length)

    if IsTrue then
        Length = Length + 240
    end
    file:write(Length..",")
 end

-- 使用uint8行程编码
-- 为稀疏细胞的优化:0到240为false；241到255的部分减240为true
local Trav_previous = false
local Trav_count = 1
for i = 1, count do
    local current = (data[i] == 1)
    if i ~= 1 then
        if current == Trav_previous then
            if (Trav_count == 239 and not current) or (Trav_count == 14 and current) then
                    writeRLE(current, Trav_count+1)
                    Trav_count = 0
            else
                Trav_count = Trav_count + 1
            end
        else
            writeRLE(Trav_previous, Trav_count)
            Trav_count = 1
        end
    end
    Trav_previous = current
    if i==count then
        writeRLE(Trav_previous, Trav_count)
    end
end

file:close()

local log = io.open("ExportLog.log", "a")
log:write(os.date(),":  Exported  ", "Width:", wd, "  Height:", ht, "\n")
log:close()