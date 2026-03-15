circle = { 
  x = 200, 
  y = 200, 
  r = 30, 
  speed = 200 }

function init()
  -- optional init logic
end

function update(dt)
  if IsKeyDown(KEY_LEFT)  then circle.x = circle.x - circle.speed * dt end
  if IsKeyDown(KEY_RIGHT) then circle.x = circle.x + circle.speed * dt end
  if IsKeyDown(KEY_UP)    then circle.y = circle.y - circle.speed * dt end
  if IsKeyDown(KEY_DOWN)  then circle.y = circle.y + circle.speed * dt end
end

function draw()
  ClearBackgroundRGBA(20, 20, 30, 255)
  DrawCircle(circle.x, circle.y, circle.r)
end
