const jwt_decode = require('jwt-decode');
class UserParser{
	async decode(token){
		return jwt_decode(token);
	}
	async getUser(token){
		const token_decoded = await this.decode(token);
		const user_id = token_decoded['user_id'];
		return user_id;
	}
}
module.exports = new UserParser();