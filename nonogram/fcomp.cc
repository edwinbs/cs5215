typedef enum
{
    state_invalid,
    state_sliding,
    state_drawing,
    state_restoring,
    state_valid,
    state_finish
} state_t;



void Infer()
{
    Init();
    
    state_t mode = state_invalid;
    while (mode != state_finish)
        mode = DecideNextState(mode);
}

state_t DecideNextState(state_t curState)
{
    switch (mode)
    {
    case state_invalid:
        mode = InvalidImpl();
        break;
    case state_valid:
        mode = ValidImpl();
        break;
    case state_sliding:
        mode = SlidingImpl();
        break;
    case state_drawing:
        mode = DrawingImpl();
        break;
    case state_restoring:
        mode = RestoringImpl();
        break;
    }
}

state_t InvalidImpl()
{
    if (b >= max_b)
        return state_valid;

    if (pos[b] + rule[b] > limit[b])
        return state_restoring;
        
    solid[b] = -1;
    for (size_t i=0; i<rule[b]; ++i)
    {
        if (cell[pos[b]] == ts_false)
        {
            if (solid[b] != -1)
            {
                target = b;
                return state_drawing;
            }
            
            pos[b] += i + 1;
            return state_invalid;
        }
        
        if (solid[b] == -1 && cell[pos[b] + i] == ts_true)
            solid[b] = i;
    }
    
    if (solid[b] != -1 && cell[pos[b] + rule[b]] == ts_true)
        solid[b] = rule[b];
        
    while (pos[b] + rule[b] < limit[b] &&
           cell[pos[b] + rule[b]] == ts_true)
    {
        if (solid[b] == 0)
        {
            target = b;
            return state_drawing;
        }
        
        ++pos[b];
        --solid[b];
    }
    
    if (b + 1 < max_b && pos[b + 1] < pos[b] + rule[b] + 1)
        pos[b + 1] = pos[b] + rule[b] + 1;
    
    ++b;
    return state_invalid;
}

state_t ValidImpl()
{
    if (b <= min_b)
        return state_finish;
    
    --b;
    
    for (size_t i = pos[b] + rule[b]; i < max_pos; ++i)
    {
        if (cell[i] == ts_true)
        {
            if (pos[b] + solid[b] + rule[b] > i)
            {
                pos[b] = i + 1 - rule[b];
                return state_invalid;
            }
            
            target = b;
            return state_drawing;
        }
        
        if (Record())
            return state_finish;
            
        min_inv = max;
        return state_sliding;
    }
}

state_t SlidingImpl()
{
    size_t limit = 0;
    if (b == max_b)
        limit = line_length;
    else if (col[b] == col[b + 1])
        limit = pos[b + 1] - 1;
    else
        limit = pos[b + 1];
        
    while (pos[b] + rule[b] < limit &&
           cell[pos[b] + rule[b]] != ts_false &&
           solid[b] != 0)
    {
        if (solid[b] > 0)
            --solid[b];
        else
            solid[b] = len + 1;
        ++pos[b];
    }
    
    if (pos[b] != oldpos[b])
    {
        if (merge1)//TODO: merge
            return state_finish;
    }
    
    if (pos[b] + rule[b] == limit && b + 1 == max_b)
    {
        if (max_b == base)
            return state_finish;
            
        --max_b;
        
        max_pos = pos[b] - 1;
    }
    else if (pos[b] + rule[b] < limit && cell[pos[b] + rule[b]] == ts_false)
    {
        size_t at = pos[b] + rule[b] + 1;
        if (pos[b] + rule[b] * 2 < limit && can_jump(a
    }
}

state_t DrawingImpl()
{
    do
    {
        if (b <= base)
            return state_finish;
            
        if (min_inv < max)
        {
            if (b == min_inv)
            {
                b = max - 1;
                return state_restoring;
            }
        }
        
        if (solid[b] < rule[b])
            target = b;
        --b;
    } while (solid[b] < rule[b] &&
             pos[target] + solid[target] - rule[b] + 1 > pos[b] + solid[b]);
             
    if (b < min_inv)
        min_inv = b;
    
    pos[b] = pos[target] + solid[target] - rule[b] + 1;
    
    return state_invalid;
}

state_t RestoringImpl()
{
    for (size_t i = b+1; i > min_inv; --i)
    {
        pos[i-1] = oldpos[i-1];
        solid[i-1] = oldsolid[i-1];
        if (solid[i-1] != -1)
            target = i-1;
    }
    b = min_inv;
    return state_drawing;
}
