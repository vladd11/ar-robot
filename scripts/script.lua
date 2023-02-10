function calcDistance(x, y, x1, y1)
    x = x - x1
    y = y - y1

    x = x * x
    y = y * y
    return math.sqrt(x + y)
end

graph = createGraph(20)

prev = 0
prevTime = 0
curr = -2
path = {}

-- Triangle matrix. Like (M(3)):
-- 1    0  0
-- nil  1  0
-- nil nil 1
matrix = {}

function appendMatrixTo(n)
    for i = #matrix + 1, n do
        matrix[i] = {}
        for j = 1, i do
            matrix[i][j] = 0
        end
    end
end

function map(x, in_min, in_max, out_min, out_max)
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
end

function turnToAngleTick(angle)
    if math.abs(angle) > 0.314 and (prev ~= 3 or prev ~= 4 or prev ~= 0) then
        if angle > 0 then
            if prev ~= 1 then
                send("S2310")
                send("E1010")
                setText("R")
                prev = 1
                prevTime = os.clock()
            end
        else
            if prev ~= 2 then
                send("S1000")
                send("E0110")
                setText("L")
                prev = 2
                prevTime = os.clock()
            end
        end
    elseif math.abs(angle) > 0.1 and (prev == 0 or prev == 3 or prev == 4) then
        if (os.clock() - prevTime) > 0.250 then
            if angle > 0 then
                send("S" .. tostring(map(angle, 0.314, 0.05, 2000, 1650)))
                setText("R " .. tostring(map(angle, 0.314, 0.05, 2000, 1650)))
                prev = 3
                prevTime = os.clock()
            else
                send("S" .. tostring(map(math.abs(angle), 0.314, 0.05, 1000, 1650)))
                setText("L " .. tostring(map(math.abs(angle), 0.314, 0.05, 1000, 1650)))
                prev = 4
                prevTime = os.clock()
            end
        end
    elseif prev ~= 0 then
        send("S1650")
        send("E0110")
        setText("F")
        prev = 0
        prevTime = os.clock()
    end
    return math.abs(angle) < 0.314
end

function tick()
    line = newLine()
    for i = 0, getAnchorsCount() - 1 do
        count = getAdjCount(graph, i)
        for j = 0, count - 1 do
            pushLine(line, i)
            pushLine(line, getAdj(graph, i, j))

            local _, _, _, _, anchorX, _, anchorZ = anchorPose(getAdj(graph, i, j))
            local _, _, _, _, cameraX, _, cameraZ = anchorPose(i)
            local distance = calcDistance(anchorX, anchorZ, cameraX, cameraZ)
            setAdjWeight(graph, i, j, distance)
        end
    end
    drawLine(line)

    if curr == -1 then
        if prev == 999 and os.clock() - prevTime > 4 then
            send("E1001")
            setText("B")
            prev = 998
            prevTime = os.clock()
        end
        if prev == 998 and os.clock() - prevTime > 4 then
            send("E1111")
            setText("S")
            prev = 998
            prevTime = os.clock()
        end
        return
    end

    if curr < 0 then
        return
    end

    if curr >= #path then
        if prev ~= 999 then
            prev = 999
            prevTime = os.clock()
            send("E1111")
            send("C1440")
            send("S1650")
            setText("S")
            curr = -1
        end
        return
    end

    local angle = angleToAnchor(path[curr + 1])
    local _, _, _, _, anchorX, _, anchorZ = anchorPose(path[curr + 1])
    local _, _, _, _, cameraX, _, cameraZ = cameraPose()
    local distance = calcDistance(anchorX, anchorZ, cameraX, cameraZ)

    turnToAngleTick(angle)

    if distance < 0.4 then
        setColor(path[curr + 1], 0, 0, 0)
        curr = curr + 1
        prev = -1
        prevTime = os.clock()
        return
    end
end

lastIdx = 999
function touch(idx)
    if lastIdx ~= 999 then
        connect(graph, idx, lastIdx, 4)
        lastIdx = 999
    else
        lastIdx = idx
    end
end
