-- lc3 assembler

local codes = {}

local op_tbl = {
    ["br"]   = 0x0, -- 0000
    ["add"]  = 0x1, -- 0001
    ["ld"]   = 0x2, -- 0010
    ["st"]   = 0x3, -- 0011
    ["jsr"]  = 0x4, -- 0100
    ["jsrr"] = 0x4, -- 0100
    ["and"]  = 0x5, -- 0101
    ["ldr"]  = 0x6, -- 0110
    ["str"]  = 0x7, -- 0111
    ["rti"]  = 0x8, -- 1000
    ["not"]  = 0x9, -- 1001
    ["ldi"]  = 0xA, -- 1010
    ["sti"]  = 0xB, -- 1011
    ["jmp"]  = 0xC, -- 1100
    ["ret"]  = 0xC, -- 1100
    ["lea"]  = 0xE, -- 1110
    ["trap"] = 0xF, -- 1111
    -- not 1101
}

-- translation
local asm = {
    t_add = function (flag, dr, sr1, sr2, imm5)
        if flag then
            return ((op_tbl["add"] << 12) | (dr << 9) | (sr1 << 6) | (1 << 5) | (imm5 << 0))
        else
            return ((op_tbl["add"] << 12) | (dr << 9) | (sr1 << 6) | (0 << 3) | (sr2 << 0))
        end
    end
}

local function parse_inst(inst)
    if not op_tbl[inst.opcode] then
        print("Invalid opcode: " .. inst.opcode)
        os.exit(1)
    end

    -- debug
    print("opcode: " .. inst.opcode)
    print("operands: ")
    for idx, operand in ipairs(inst.operands) do
        print("", idx, operand)
    end
end

local function parse_line(line)
    local opcode = string.match(line, "^%s*(%w+)")
    line = string.gsub(line, "^%s*(%w+)%s*", "")

    local operands = {}
    for operand in string.gmatch(line, "%s*(%w+)") do
        table.insert(operands, operand)
    end

    local inst = {}
    inst.opcode = opcode
    inst.operands = operands

    parse_inst(inst)
end

local function run()
    if not arg[1] then
        print("Need to a input file")
        os.exit(1)
    end

    local file = assert(io.open(arg[1], "r"))

    for line in file:lines() do
        if line and line ~= "" then
            parse_line(line)
        end
    end
end

run()
