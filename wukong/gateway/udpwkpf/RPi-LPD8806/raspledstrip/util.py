def drange(start, stop, step):
    r = start
    while r < stop:
        yield r
        r += step

def wave_range(start, peak, step):
    main = [i for i in drange(start, peak, step)]
    return main + [i for i in reversed(main[0:len(main)-1])]

