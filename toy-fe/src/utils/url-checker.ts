import getAPI from 'toy-api-wrapper';

const api = getAPI();

export default async (url: string, method: string): Promise<boolean> => {
    try {
        await api.server.request(url, {
            method,
        });
        return true;
    } catch (error) {
        return ![0, 404].includes(error.code);
    }
};
