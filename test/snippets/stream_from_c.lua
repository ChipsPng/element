result = ""
local function read_and_print_input()
    result = io.read ("a")
    print ("data read: " .. result)
end

currentpos = stream:seek ("cur")
local oi = io.input()
io.input (stream)

read_and_print_input()

io.input (oi)
stream:close()
stream = nil
