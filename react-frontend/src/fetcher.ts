export const fetchToESP = async (
  callback: string,
): Promise<Response> => {
  return fetch(`/${callback}`, {
    method: 'POST',
  });
}