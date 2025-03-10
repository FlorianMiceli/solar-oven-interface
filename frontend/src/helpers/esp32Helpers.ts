export const esp32ApiGet = async (endpoint: string) => {
    return await fetch(`http://192.168.4.1/${endpoint}`,{
        method: 'GET',
        mode: 'no-cors'
    })
}