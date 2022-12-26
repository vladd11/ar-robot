function calcDistance(x, y, x1, y1)
    x = x - x1
    y = y - y1

    x = x * x
    y = y * y
    return math.sqrt(x + y)
end

prev = 0
curr = 0

function tick()
    if curr >= getAnchorsCount() then
        if prev ~= -1 then
            prev = -1
            send("S")
            setText("S")
        end
        return
    end

    local angle = angleToAnchor(curr)
    local _, _, _, _, anchorX, _, anchorZ = anchorPose(curr)
    local _, _, _, _, cameraX, _, cameraZ = cameraPose()
    local distance = calcDistance(anchorX, anchorZ, cameraX, cameraZ)

    if distance < 0.6 then
        setColor(curr, 0, 0, 0)
        curr = curr + 1
        return
    end

    if math.abs(angle) > 0.314 then
        if angle > 0 then
            if prev ~= 1 then
                send("R")
                setText("R")
                prev = 1
            end
        else
            if prev ~= 2 then
                send("L")
                setText("L")
                prev = 2
            end
        end
    elseif prev ~= 0 then
        send("F")
        setText("F")
        prev = 0
    end
end