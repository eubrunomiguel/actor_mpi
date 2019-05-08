import Foundation

extension String {
    func padLeft (totalWidth: Int, with: String) -> String {
        let toPad = totalWidth - self.characters.count
        if toPad < 1 { return self }
        return "".padding(toLength: toPad, withPad: with, startingAt: 0) + self
    }
}

public func printArray(xSize:Int, ySize:Int, arr:[[Int]], places:Int) {
    let digits = Int(log10(Double(places)).rounded(.up))
    for y in 0 ..< ySize {
        for x in 0 ..< xSize {
            print("\("\(arr[y][x])".padLeft(totalWidth: digits, with: "0")) ", separator:"", terminator:"")
        }
        print()
    }
}

public func getEdgeCut(arrIn:inout [[Int]], ySize:Int, xSize:Int) -> Int {
    var res = 0
    for y in 0 ..< ySize {
        for x in 1 ..< xSize {
            res += ((arrIn[y][x-1] != arrIn[y][x]) ? 1 : 0)
        }
    }
    for y in 1 ..< ySize {
        for x in 0 ..< xSize {
            res += ((arrIn[y-1][x] != arrIn[y][x]) ? 1 : 0)
        }
    }
    return res
}

func getLocalEdgeCut(arrIn:[[Int]], ySize:Int, xSize:Int, x:Int, y:Int, withVal:Int) -> Int {
    var res = (y > 0 && arrIn[y-1][x] != withVal) ? 1 : 0
    res += (y < ySize - 1 && arrIn[y+1][x] != withVal) ? 1 : 0
    res += (x > 0 && arrIn[y][x-1] != withVal) ? 1 : 0
    res += (x < xSize - 1 && arrIn[y][x+1] != withVal) ? 1 : 0
    return res
}

public func changeRandom(arr:inout [[Int]], ySize:Int, xSize:Int, n:Int) {
    var changesLeft = n
    while changesLeft > 0 {
        let x = Int(arc4random()) % xSize
        let y = Int(arc4random()) % ySize
        let r = arc4random() 
        if y > 0 && r == 0 && arr[y][x] != arr[y-1][x] {
            arr[y][x] = arr[y-1][x]
            changesLeft -= 1
        } else if y < ySize - 1 && r == 1 && arr[y][x] != arr[y+1][x] {
            arr[y][x] = arr[y+1][x]
            changesLeft -= 1
        } else if x > 0 && r == 3 && arr[y][x] != arr[y][x-1] {
            arr[y][x] = arr[y][x-1]
            changesLeft -= 1
        } else if x < xSize - 1 && r == 4 && arr[y][x] != arr[y][x+1] {
            arr[y][x] = arr[y][x+1]
            changesLeft -= 1
        }
    }
}

public func createNCopies(arr:inout [[Int]], n:Int) -> [[[Int]]] {
    return [[[Int]]](repeating: arr, count: n)
}

public func getDisparity(arr:inout[[Int]], places:Int) -> Int {

    let optimal = Double(arr.count * arr[0].count) / Double(places)
    var buckets = Dictionary<Int, Double>()
    for row in arr {
        for i in row {
            if let x = buckets[i] {
                buckets[i] = x + 1.0
            } else {
                buckets[i] = 1.0
            }
        }
    }

    var missingPenalty = 0
    for i in 0 ..< places {
        if buckets[i] == nil {
            buckets[i] = 0.0
            missingPenalty += 500
        }
    }
    let disparity = buckets.values.reduce(0.0, {(result:Double, placeCount:Double) in result + abs(placeCount - optimal)})

    return Int(disparity) + missingPenalty
}

public func fitness(disparity:Int, disparityWeight:Int, cut:Int, cutWeight:Int) -> Int {
    return disparity * disparityWeight + cut * cutWeight
}

public func performIteration(iteration:Int, arrays:inout [[[Int]]], ySize:Int, xSize:Int, places:Int, disparityWeight:Int, cutWeight:Int, genBase:Int, previousBest:Int) -> (Int, Int) {
    for i in 0 ..< 4 * genBase {
        if (i != previousBest) {
            changeRandom(arr: &arrays[i], ySize: ySize, xSize: xSize, n: iteration/10+2)
        }
    }

    var best = Int.max
    var bestIdx = -1
    var bestDisparity = Int.max
    var bestDisparityIdx = -1
    var bestEdgeCut = Int.max
    var bestEdgeCutIdx = -1


    for i in 0 ..< 4 * genBase {
        let disparity = getDisparity(arr: &arrays[i], places:places)
        let edgeCut = getEdgeCut(arrIn: &arrays[i], ySize: ySize, xSize: xSize)
        let fitnessVal = fitness(disparity:disparity, disparityWeight:disparityWeight, cut:edgeCut, cutWeight:cutWeight)
        if fitnessVal < best {
            best = fitnessVal
            bestIdx = i
        }
        if disparity < bestDisparity {
            bestDisparity = disparity
            bestDisparityIdx = i
        }
        if edgeCut < bestEdgeCut {
            bestEdgeCut = edgeCut
            bestEdgeCutIdx = i
        }
    }

    let disparity0 = getDisparity(arr: &arrays[0], places:places)
    let edgeCut0 = getEdgeCut(arrIn: &arrays[0], ySize: ySize, xSize: xSize)
    let fitnessVal0 = fitness(disparity:disparity0, disparityWeight:disparityWeight, cut:edgeCut0, cutWeight:cutWeight)

    var i = 0
    var copiesLeft = 2 * genBase - 1
    while copiesLeft > 0 {
        if i != bestIdx && i != bestEdgeCutIdx && i != bestDisparityIdx {
            arrays[i] = arrays[bestIdx]
            copiesLeft -= 1
        }
        i += 1
    }
    copiesLeft = genBase - 1
    while copiesLeft > 0 {
        if i != bestIdx && i != bestEdgeCutIdx && i != bestDisparityIdx {
            arrays[i] = arrays[bestDisparityIdx]
            copiesLeft -= 1
        }
        i += 1
    }
    copiesLeft = genBase - 1
    while copiesLeft > 0 {
        if i != bestIdx && i != bestEdgeCutIdx && i != bestDisparityIdx {
            arrays[i] = arrays[bestEdgeCutIdx]
            copiesLeft -= 1
        }
        i += 1
    }
    return (best, bestIdx)
}


func swapBoundary(biggest:Int, smallest:Int,  arrIn:inout [[Int]], occs:inout [Int], ySize:Int, xSize:Int) -> Int {
    if (occs[biggest] <= occs[smallest]) {
        return 0
    }

    var changesMade = 0
    for y in 0 ..< ySize {
        for x in 0 ..< xSize {
            let curCount = occs[arrIn[y][x]]
            let curPlace = arrIn[y][x]
            let curCut = getLocalEdgeCut(arrIn: arrIn, ySize: ySize, xSize: xSize, x: x, y: y, withVal:arrIn[y][x])
            if y > 0 && occs[arrIn[y-1][x]] < curCount - 1 && getLocalEdgeCut(arrIn: arrIn, ySize: ySize, xSize: xSize, x: x, y: y, withVal:arrIn[y-1][x]) <= curCut {


                arrIn[y][x] = arrIn[y-1][x]
                occs[arrIn[y-1][x]] += 1
                occs[curPlace] -= 1
                changesMade += 1
                continue
            }
            if y < arrIn.count - 1 && occs[arrIn[y+1][x]] < curCount - 1 && getLocalEdgeCut(arrIn: arrIn, ySize: ySize, xSize: xSize, x: x, y: y, withVal:arrIn[y+1][x]) <= curCut {
                arrIn[y][x] = arrIn[y+1][x]
                occs[arrIn[y+1][x]] += 1
                occs[curPlace] -= 1
                changesMade += 1
                continue
            }
            if x > 0 && occs[arrIn[y][x-1]] < curCount - 1 && getLocalEdgeCut(arrIn: arrIn, ySize: ySize, xSize: xSize, x: x, y: y, withVal:arrIn[y][x-1]) <= curCut {
                arrIn[y][x] = arrIn[y][x-1]
                occs[arrIn[y][x-1]] += 1
                occs[curPlace] -= 1
                changesMade += 1
                continue
            }
            if x < arrIn[y].count - 1  && occs[arrIn[y][x+1]] < curCount - 1 && getLocalEdgeCut(arrIn: arrIn, ySize: ySize, xSize: xSize, x: x, y: y, withVal:arrIn[y][x+1]) <= curCut {
                arrIn[y][x] = arrIn[y][x+1]
                occs[arrIn[y][x+1]] += 1
                occs[curPlace] -= 1
                changesMade += 1
                continue
            }
        }
    }
    return changesMade
}

public func normalizeSizes(arrIn:inout [[Int]], ySize:Int, xSize:Int, places:Int) {
    var counts = [Int](repeating:0, count:places)
    for y in 0 ..< ySize {
        for x in 0 ..< xSize {
            counts[arrIn[y][x]] = counts[arrIn[y][x]] + 1
        }
    }

    var maxIdx = -1
    var minIdx = -1
    var max = -1
    var min = xSize * ySize + 1
    var changesMade = 0

    repeat {
        maxIdx = -1
        minIdx = -1
        max = -1
        min = Int.max

        for i in 0 ..< counts.count {
            if min > counts[i] {
                minIdx = i
                min = counts[i]
            }
            if max < counts[i] {
                maxIdx = i
                max = counts[i]
            }
        }
        changesMade = swapBoundary(biggest: maxIdx, smallest: minIdx, arrIn: &arrIn, occs: &counts, ySize:ySize, xSize:xSize)
        print(counts)
        print("Changes made: \(changesMade)")

    } while min < max - 1 && changesMade != 0
}

public func putInitial(arrIn:inout [[Int]], ySize:Int, xSize:Int, ySplits:Int, xSplits:Int, places:Int) {
    for y in 0 ..< ySize {
        for x in 0 ..< xSize {
//            let tmpx = x / (xSize/xSplits)
//            let tmpy = y / (ySize/ySplits)
//            let res = tmpy * xSplits + tmpx
            arrIn[y][x] = Int(arc4random()) % places
        }
    }
}

public func gcd(_ m: Int, _ n: Int) -> Int {
    var a = 0
    var b = max(m, n)
    var r = min(m, n)

    while r != 0 {
        a = b
        b = r
        r = a % b
    }
    return b
}


