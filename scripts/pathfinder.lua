prev = 0

function tick()
    local angle = angleToAnchor(0)
    if math.abs(angle) > 0.314 then
        if dif > 0 then
            if prev ~= 1 then
                send("right")
                prev = 1
            end
        else
            if prev ~= 2 then
                send("left")
                prev = 2
            end
        end
    elseif prev ~= 0 then
        send("")
        prev = 0
    end
end